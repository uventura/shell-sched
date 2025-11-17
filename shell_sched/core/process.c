// shell_sched/core/process.c
#define _POSIX_C_SOURCE 200809L

#include "shell_sched/core/process.h"
#include "shell_sched/core/common.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define QUANTUM_SECONDS 20
#define SLEEP_STEP_NS 200000000L // 200ms

typedef struct job_node {
    pid_t pid;
    int priority;
    char command[SHELL_SCHED_CMD_MAX];
    int remaining_quantum; // in seconds (remaining)
    struct job_node* next;
} job_node;

typedef struct queue {
    job_node* head;
    job_node* tail;
    int count;
} queue;

static void queue_init(queue* q) { q->head = q->tail = NULL; q->count = 0; }
static void queue_push_tail(queue* q, job_node* n) {
    n->next = NULL;
    if (!q->tail) q->head = q->tail = n;
    else { q->tail->next = n; q->tail = n; }
    q->count++;
}
static void queue_push_head(queue* q, job_node* n) {
    if (!q->head) { n->next = NULL; q->head = q->tail = n; }
    else { n->next = q->head; q->head = n; }
    q->count++;
}
static job_node* queue_pop_head(queue* q) {
    if (!q->head) return NULL;
    job_node* r = q->head;
    q->head = r->next;
    if (!q->head) q->tail = NULL;
    r->next = NULL;
    q->count--;
    return r;
}
static void queue_remove_by_pid(queue* q, pid_t pid) {
    job_node* prev = NULL;
    job_node* cur = q->head;
    while (cur) {
        if (cur->pid == pid) {
            if (prev) prev->next = cur->next;
            else q->head = cur->next;
            if (cur == q->tail) q->tail = prev;
            free(cur);
            q->count--;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static int msqid_global = -1;
static int scheduler_queues = 0;

// helper: create job_node from message (no pid yet)
static job_node* job_node_from_msg(const ShellSchedMsgNewProcess* msg) {
    job_node* n = calloc(1, sizeof(job_node));
    if (!n) return NULL;
    n->pid = -1;
    n->priority = msg->priority;
    n->remaining_quantum = QUANTUM_SECONDS;
    strncpy(n->command, msg->command, SHELL_SCHED_CMD_MAX-1);
    n->command[SHELL_SCHED_CMD_MAX-1] = '\0';
    n->next = NULL;
    return n;
}

// non-blocking check for new messages; returns 1 if message was received
static int try_receive_new_process_msg(queue queues[]) {
    ShellSchedMsgNewProcess msg;
    ssize_t r = msgrcv(msqid_global, &msg, sizeof(ShellSchedMsgNewProcess) - sizeof(long),
                       SHELL_SCHED_MSG_NEW_PROCESS, IPC_NOWAIT);
    if (r == -1) {
        if (errno == ENOMSG) return 0;
        // other error
        perror("[Scheduler] msgrcv");
        return 0;
    }
    if (msg.priority < 1 || msg.priority > scheduler_queues) {
        fprintf(stderr, "[Scheduler] Received invalid priority %d\n", msg.priority);
        return 0;
    }
    job_node* n = job_node_from_msg(&msg);
    if (!n) {
        fprintf(stderr, "[Scheduler] malloc failure\n");
        return 0;
    }
    // enqueue at tail of its priority queue (priority 1 -> index 0)
    int idx = msg.priority - 1;
    queue_push_tail(&queues[idx], n);
    return 1;
}

// spawn a new child for job (fresh)
static pid_t spawn_job_process(const char* command) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("[Scheduler] fork");
        return -1;
    }
    if (pid == 0) {
        // child: exec the command (assume it's a path or name in PATH)
        // NOTE: child runs briefly until SIGSTOP arrives; that's fine.
        execlp(command, command, (char*)NULL);
        // if execlp fails:
        perror("[Child] execlp");
        _exit(127);
    }
    // parent: stop the child right away so it doesn't run concurrently
    if (kill(pid, SIGSTOP) == -1) {
        // If SIGSTOP fails, still return pid but warn
        perror("[Scheduler] kill(SIGSTOP) after fork");
    }
    return pid;
}

// check terminated children and remove from queues if necessary
static void reap_children_and_clean(queue queues[]) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // remove from any queue (in case it was enqueued)
        for (int i = 0; i < scheduler_queues; ++i) {
            queue_remove_by_pid(&queues[i], pid);
        }
        // also print info
        if (WIFEXITED(status)) {
            printf("[Scheduler] PID %d exited (status=%d)\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[Scheduler] PID %d killed by signal %d\n", pid, WTERMSIG(status));
        } else {
            printf("[Scheduler] PID %d changed state\n", pid);
        }
    }
}

// main scheduler loop (called in child process after create_create_user_scheduler)
void shell_sched_scheduler_main(key_t key, int flags, int queues) {
    scheduler_queues = queues;
    msqid_global = msgget(key, flags);
    if (msqid_global < 0) {
        perror("[Scheduler] msgget");
        exit(EXIT_FAILURE);
    }
    printf("[Scheduler] started (msqid=%d) with %d queue(s)\n", msqid_global, scheduler_queues);
    // Make scheduler prints line-buffered so prints appear in real time
    setvbuf(stdout, NULL, _IOLBF, 0);


    queue *q_arr = calloc(scheduler_queues, sizeof(queue));
    if (!q_arr) { perror("[Scheduler] calloc"); exit(EXIT_FAILURE); }
    for (int i = 0; i < scheduler_queues; ++i) queue_init(&q_arr[i]);

    job_node* running = NULL;

    struct timespec sleep_step;
    sleep_step.tv_sec = 0;
    sleep_step.tv_nsec = SLEEP_STEP_NS;

    while (1) {
        // 1) reap any finished children
        reap_children_and_clean(q_arr);

        // 2) pull all pending messages (but do not block) and enqueue them
        int any_msg = 0;
        do {
            any_msg = try_receive_new_process_msg(q_arr);
            if (any_msg) {
                // If new message arrived and its priority is higher than running, we will handle preemption below
            }
        } while (any_msg);

        // 3) Preemption: if running exists and there's a higher-priority job in queues, preempt
        if (running) {
            int running_prio_idx = running->priority - 1;
            int found_higher = 0;
            for (int i = 0; i < running_prio_idx; ++i) {
                if (q_arr[i].count > 0) { found_higher = 1; break; }
            }
            if (found_higher) {
                // stop running process
                if (kill(running->pid, SIGSTOP) == -1) {
                    perror("[Scheduler] kill(SIGSTOP)");
                } else {
                    // place running at *head* of its queue, preserving remaining quantum
                    queue_push_head(&q_arr[running_prio_idx], running);
                    running = NULL;
                    // continue to next iteration to pick highest-priority job
                    continue;
                }
            }
        }

        // 4) If no running process, pick highest priority non-empty queue
        if (!running) {
            job_node* pick = NULL;
            // int pick_idx = -1;
            for (int i = 0; i < scheduler_queues; ++i) {
                if (q_arr[i].count > 0) {
                    pick = queue_pop_head(&q_arr[i]);
                    // pick_idx = i;
                    break;
                }
            }
            if (pick) {
                if (pick->pid == -1) {
                    pid_t pid = spawn_job_process(pick->command);
                    if (pid == -1) {
                        free(pick);
                        continue;
                    }
                    pick->pid = pid;
                    // child was stopped by spawn_job_process; now resume it to start running
                    if (kill(pick->pid, SIGCONT) == -1) {
                        perror("[Scheduler] kill(SIGCONT) after spawn");
                    }
                } else {
                    // was stopped previously -> resume
                    if (kill(pick->pid, SIGCONT) == -1) {
                        perror("[Scheduler] kill(SIGCONT)");
                    }
                }
                running = pick;
                
            } else {
                // nothing to run; sleep a bit and loop
                nanosleep(&sleep_step, NULL);
                continue;
            }
        }

        struct timespec start, now;
        clock_gettime(CLOCK_MONOTONIC, &start);
        while (running) {
            // check if child terminated
            pid_t w = waitpid(running->pid, NULL, WNOHANG);
            if (w == running->pid) {
                // process ended while running
                printf("[Scheduler] PID %d finished while running\n", running->pid);
                free(running);
                running = NULL;
                break;
            }

            // check for new messages (and enqueue them)
            try_receive_new_process_msg(q_arr);

            // check preemption (higher-priority arrival) - same logic as above
            int running_prio_idx = running->priority - 1;
            int found_higher = 0;
            for (int i = 0; i < running_prio_idx; ++i) {
                if (q_arr[i].count > 0) { found_higher = 1; break; }
            }
            if (found_higher) {
                // stop running and place at head preserving remaining quantum
                if (kill(running->pid, SIGSTOP) == -1) perror("[Scheduler] kill(SIGSTOP)");
                else {
                    queue_push_head(&q_arr[running_prio_idx], running);
                    running = NULL;
                    break;
                }
            }

            // sleep one step
            nanosleep(&sleep_step, NULL);

            // decrement remaining quantum - compute elapsed since start
            clock_gettime(CLOCK_MONOTONIC, &now);
            double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
            // compute remaining as integer seconds
            int remaining_after_elapsed = running->remaining_quantum - (int)elapsed;
            if (remaining_after_elapsed < 0) remaining_after_elapsed = 0;
            running->remaining_quantum = remaining_after_elapsed;

            // if quantum expired:
            if (running->remaining_quantum <= 0) {
                // if process still running and not finished, stop it and put to tail (RR)
                if (kill(running->pid, SIGSTOP) == -1) {
                    // might have finished between checks
                    if (errno != ESRCH) perror("[Scheduler] kill(SIGSTOP) on quantum end");
                } else {
                    // reset quantum for future
                    running->remaining_quantum = QUANTUM_SECONDS;
                    int idx = running->priority - 1;
                    queue_push_tail(&q_arr[idx], running);
                    running = NULL;
                }
                break;
            }

            // also check if child finished inside this short interval
            int status;
            pid_t rch = waitpid(running->pid, &status, WNOHANG);
            if (rch == running->pid) {
                // child ended
                if (WIFEXITED(status)) {
                    printf("[Scheduler] PID %d exited (status=%d)\n", running->pid, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("[Scheduler] PID %d killed by signal %d\n", running->pid, WTERMSIG(status));
                }
                free(running);
                running = NULL;
                break;
            }
        } // end while running loop
    } // end main loop

    // unreachable, but free if done
    for (int i = 0; i < scheduler_queues; ++i) {
        job_node* j = q_arr[i].head;
        while (j) { job_node* tmp = j->next; free(j); j = tmp; }
    }
    free(q_arr);
}
