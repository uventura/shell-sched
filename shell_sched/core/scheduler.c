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
#include <signal.h>

#define SCHEDULER_QUANTUM 20

void init_scheduler_queues(void);
void execute_process_scheduler(int signal);
void continue_parent_process(void);
void destroy_scheduler(int signal);

int scheduler_shared_memory_id;
ShellSchedSharedMemData* scheduler_shared_memory;
ShellSchedScheduler scheduler;

/* === Helper functions === */

static ShellSchedProcess* scheduler_pick_next(void) {
    for (int i = 0; i < scheduler.queues; ++i) {
        ShellSchedProcessQueue* q = &scheduler.process_queue[i];
        if (q->size > 0) {
            return shell_sched_process_queue_pop(q);
        }
    }
    return NULL;
}

static void scheduler_requeue_process(ShellSchedProcess* p) {
    if (!p) return;
    int prio = p->priority;

    if (prio < 0 || prio >= scheduler.queues) {
        prio = scheduler.queues - 1;
    }
    shell_sched_process_queue_push(&scheduler.process_queue[prio], p);
}

/* ========================= */

void shell_sched_init_scheduler() {
    printf("Starting scheduler...\n");
    signal(SIGQUIT, destroy_scheduler);
    signal(SIGUSR1, execute_process_scheduler);

    scheduler_shared_memory = shell_sched_attach_shared_memory();
    ShellSchedSharedMemData data = shell_sched_read_shared_memory(scheduler_shared_memory);

    scheduler.queues = data.i32;
    if (scheduler.queues <= 0 || scheduler.queues > 3) {
        shell_sched_throw_execution_error("[ShellSchedError] The number of queues can be 2 or 3.\n");
        return;
    }

    printf("Configured queues: %d\n", scheduler.queues);

    init_scheduler_queues();
    scheduler.started = true;
    printf("Scheduler started.\n");

    continue_parent_process();
}

void shell_sched_run_scheduler() {
    printf("Running scheduler...\n");

    while (1) {
        ShellSchedProcess* p = scheduler_pick_next();

        if (!p) {
            sleep(1);
            continue;
        }

        printf("[Scheduler] Running PID=%d (prio=%d)\n", p->pid, p->priority);

        if (kill(p->pid, 0) == -1 && errno == ESRCH) {
            printf("[Scheduler] PID %d no longer exists.\n", p->pid);
            free(p);
            continue;
        }

        kill(p->pid, SIGCONT);
        sleep(SCHEDULER_QUANTUM);
        kill(p->pid, SIGSTOP);

        if (kill(p->pid, 0) == -1 && errno == ESRCH) {
            printf("[Scheduler] PID %d terminated.\n", p->pid);
            free(p);
        } else {
            scheduler_requeue_process(p);
        }
    }

    exit(SHELL_SCHED_FINISHED);
}

/* === Nova implementação correta === */

void execute_process_scheduler(int signal) {
    if (!scheduler.started) {
        shell_sched_throw_execution_error(
            "[ShellSchedError] Scheduler is not started.\n"
        );
        exit(-1);
    }

    if (scheduler_shared_memory->type == NEW_PROCESS_SHARED) {

        ShellSchedNewProcess newp = scheduler_shared_memory->process;

        printf("[Scheduler] New process: priority=%d\n", newp.priority);
        printf("[Scheduler] Command: %s\n", newp.command);

        /* Criar processo real */
        pid_t pid = fork();

        if (pid == -1) {
            perror("[Scheduler] fork() failed");
            continue_parent_process();
            return;
        }

        if (pid == 0) {
            /* FILHO: executa o comando enviado */
            execl("/bin/sh", "sh", "-c", newp.command, NULL);
            perror("[Scheduler] exec failed");
            exit(1);
        }

        /* PAI: cria estrutura interna */
        ShellSchedProcess* p = malloc(sizeof(ShellSchedProcess));
        if (!p) {
            perror("[Scheduler] malloc failed");
            continue_parent_process();
            return;
        }

        p->pid = pid;
        p->priority = newp.priority;
        p->remaining = 0;
        p->next = NULL;

        if (p->priority < 0 || p->priority >= scheduler.queues)
            p->priority = scheduler.queues - 1;

        shell_sched_process_queue_push(&scheduler.process_queue[p->priority], p);

        printf("[Scheduler] Added PID=%d to queue %d\n", pid, p->priority);
    }

    continue_parent_process();
}

void destroy_scheduler(int signal) {
    if (scheduler.started) {
        shell_sched_dettach_shared_memory(scheduler_shared_memory);

        for (int i = 0; i < scheduler.queues; ++i) {
            ShellSchedProcessQueue* q = &scheduler.process_queue[i];
            while (q->size > 0) {
                ShellSchedProcess* p = shell_sched_process_queue_pop(q);
                if (p) free(p);
            }
            shell_sched_process_queue_free(q);
        }
    }

    printf("Scheduler destroyed.\n");
    exit(SHELL_SCHED_FINISHED);
}

void init_scheduler_queues(void) {
    for (int i = 0; i < scheduler.queues; ++i) {
        shell_sched_process_queue_init(&scheduler.process_queue[i]);
    }
}

void continue_parent_process(void) {
    int r = kill(scheduler.parent, SIGRTMIN);
    printf("Continue process called.\n");

    if (r != SHELL_SCHED_SUCCESSFULL_REQUEST) {
        shell_sched_throw_execution_error(
            "[ShellSchedError] Error in sending signal result."
        );
    }
}
