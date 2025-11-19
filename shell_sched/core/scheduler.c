#include "shell_sched/core/scheduler.h"
#include "shell_sched/core/common.h"
#include "shell_sched/core/process.h"
#include "shell_sched/core/exceptions.h"
#include "shell_sched/core/shared.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define _POSIX_C_SOURCE 200809L
// #define __USE_POSIX 200809L
#include <signal.h>

#define SCHEDULER_QUANTUM 20
#define NEW_PROCESS_ERROR -1

int scheduler_shared_memory_id;
ShellSchedSharedMemData* scheduler_shared_memory;
ShellSchedScheduler scheduler;

void init_scheduler_queues(void);
void execute_process_scheduler(int signal);
void list_scheduler_processes(int signal);

void continue_parent_process(void);
void destroy_scheduler(int signal);

ShellSchedProcess* scheduler_pick_next(void);
void scheduler_requeue_process(ShellSchedProcess* p);

void shell_sched_init_scheduler() {
    printf("Starting scheduler...\n");
    signal(SIGQUIT, destroy_scheduler);
    signal(SIGINT, execute_process_scheduler);
    signal(SIGRTMIN, execute_process_scheduler);
    signal(SIGUSR1, list_scheduler_processes);

    scheduler_shared_memory = shell_sched_attach_shared_memory();
    ShellSchedSharedMemData data = shell_sched_read_shared_memory(scheduler_shared_memory);

    scheduler.queues = data.i32;
    if(scheduler.queues <= 0 || scheduler.queues > 3) {
        shell_sched_throw_execution_error("[ShellSchedError] The number of queues can be 2 or 3.\n");
        return;
    }

    init_scheduler_queues();
    scheduler.started = true;
    scheduler.parent = getppid();
    scheduler.running_process = NULL;
    printf("Scheduler started.\n");

    continue_parent_process();
}

void shell_sched_run_scheduler() {
    printf("Running scheduler...\n");

    while(RUNNING) {
        scheduler.running_process = scheduler_pick_next();
        if (!scheduler.running_process) {
            // Empty queue, wait for new processes
            sleep(1);
            continue;
        }

        ShellSchedProcess* process = scheduler.running_process;
        printf("[Scheduler] Running PID=%d (prio=%d)\n", process->pid, process->priority);
        if (kill(process->pid, 0) == -1 && errno == ESRCH) {
            printf("[Scheduler] PID %d no longer exists.\n", process->pid);
            free(process);
            continue;
        }

        kill(process->pid, SIGCONT);
        sleep(SCHEDULER_QUANTUM);
        kill(process->pid, SIGSTOP);

        if (kill(process->pid, 0) == -1 && errno == ESRCH) {
            printf("[Scheduler] PID %d terminated.\n", process->pid);
            free(process);
        } else {
            scheduler_requeue_process(process);
        }
    }
    exit(SHELL_SCHED_FINISHED);
}

void execute_process_scheduler(int signal) {
    if(!scheduler.started) {
        shell_sched_throw_execution_error("[ShellSchedError] The scheduler is not started, please run 'user_scheduler <queues>' first.\n\n");
        exit(-1);
    }

    if (scheduler_shared_memory->type == NEW_PROCESS_SHARED) {

        ShellSchedNewProcess new_process = scheduler_shared_memory->process;

        printf("[Scheduler] New process: priority=%d\n", new_process.priority);
        printf("[Scheduler] Command: %s\n", new_process.command);

        pid_t pid = fork();
        if (pid == NEW_PROCESS_ERROR) {
            perror("[Scheduler] fork() failed");
            continue_parent_process();
            return;
        } else if(pid == CHILD_PROCESS) {
            execl("/bin/sh", "sh", "-c", new_process.command, NULL);
            perror("[Scheduler] exec failed");
            exit(1);
        } else {
            // Start paused
            kill(pid, SIGSTOP);
        }

        ShellSchedProcess* process = malloc(sizeof(ShellSchedProcess));
        if (!process) {
            perror("[Scheduler] malloc failed");
            continue_parent_process();
            return;
        }

        process->pid = pid;
        process->priority = new_process.priority - 1;
        process->remaining = 0;
        process->next = NULL;

        if (process->priority < 0 || process->priority >= scheduler.queues) {
            process->priority = scheduler.queues - 1;
        }
        printf("Process priority adjusted to %d\n", process->priority);
        shell_sched_process_queue_push(&scheduler.process_queue[process->priority], process);
        printf("[Scheduler] Added PID=%d to queue %d\n", pid, process->priority);
    }
    continue_parent_process();
}

void list_scheduler_processes(int signal) {
    printf("\n[Scheduled processes]\n");
    for (int i = 0; i < scheduler.queues; ++i) {
        ShellSchedProcessQueue* queue = &scheduler.process_queue[i];
        printf("| Queue %d (size=%d): ", i, queue->size);

        ShellSchedProcess* current = queue->front;
        while (current) {
            printf("[PID=%d, prio=%d] -> ", current->pid, current->priority);
            current = current->next;
        }
        printf("NULL\n");
    }

    printf("\n[Running process]");
    if (scheduler.running_process) {
        printf(" [PID=%d, prio=%d]\n", scheduler.running_process->pid, scheduler.running_process->priority);
    } else {
        printf("| NULL\n");
    }
    printf("---------------------------------------------\n\n");

    continue_parent_process();
}

void destroy_scheduler(int signal) {
    if(scheduler.started) {
        shell_sched_dettach_shared_memory(scheduler_shared_memory);
        shell_sched_process_queue_free(scheduler.process_queue);

        for (int i = 0; i < scheduler.queues; ++i) {
            ShellSchedProcessQueue* queue = &scheduler.process_queue[i];
            while (queue->size > 0) {
                ShellSchedProcess* process = shell_sched_process_queue_pop(queue);
                kill(process->pid, SIGTERM);
                if(process) free(process);
            }
            shell_sched_process_queue_free(queue);
        }
    }

    printf("Scheduler destroyed.\n");
    exit(SHELL_SCHED_FINISHED);
}

void init_scheduler_queues(void) {
    for(int i = 0; i < scheduler.queues; ++i) {
        shell_sched_process_queue_init(&scheduler.process_queue[i]);
    }
}

void continue_parent_process(void) {
    int signal_result = kill(scheduler.parent, SIGRTMIN);
    if(signal_result != SHELL_SCHED_SUCCESSFULL_REQUEST) {
        shell_sched_throw_execution_error("[ShellSchedError] Error in sending signal result.");
    }
}

ShellSchedProcess* scheduler_pick_next(void) {
    for (int i = 0; i < scheduler.queues; ++i) {
        ShellSchedProcessQueue* q = &scheduler.process_queue[i];
        if (q->size > 0) {
            return shell_sched_process_queue_pop(q);
        }
    }
    return NULL;
}

void scheduler_requeue_process(ShellSchedProcess* p) {
    if (!p) return;
    int prio = p->priority;

    if (prio < 0 || prio >= scheduler.queues) {
        prio = scheduler.queues - 1;
    }
    shell_sched_process_queue_push(&scheduler.process_queue[prio], p);
}
