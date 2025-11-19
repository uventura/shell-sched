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

#define _POSIX_C_SOURCE 200809L
// #define __USE_POSIX 200809L
#include <signal.h>

#define SCHEDULER_QUANTUM 5

int scheduler_shared_memory_id;
ShellSchedSharedMemData* scheduler_shared_memory;
ShellSchedScheduler scheduler;

void init_scheduler_queues(void);
void execute_process_scheduler(int signal);
void continue_parent_process(void);
void destroy_scheduler(int signal);

void shell_sched_init_scheduler() {
    printf("Starting scheduler...\n");
    signal(SIGQUIT, destroy_scheduler);
    signal(SIGINT, execute_process_scheduler);
    signal(SIGRTMIN, execute_process_scheduler);   // handle queued realtime signals

    scheduler_shared_memory = shell_sched_attach_shared_memory();
    ShellSchedSharedMemData data = shell_sched_read_shared_memory(scheduler_shared_memory);

    scheduler.queues = data.i32;
    if(scheduler.queues <= 0 || scheduler.queues > 3) {
        shell_sched_throw_execution_error("[ShellSchedError] The number of queues can be 2 or 3.\n");
        return;
    }

    printf("%d\n", scheduler.queues);

    init_scheduler_queues();
    scheduler.started = true;
    printf("Scheduler started.\n");
    continue_parent_process();
}

void shell_sched_run_scheduler() {
    printf("Running scheduler...\n");

    while(1) {
        // Round Robin
        //----------------------
        sleep(SCHEDULER_QUANTUM);
        printf("\n<Running>\n");
    }
    printf("End of scheduler!\n");
    exit(SHELL_SCHED_FINISHED);
}

void execute_process_scheduler(int signal) {
    if(!scheduler.started) {
        shell_sched_throw_execution_error("[ShellSchedError] The scheduler is not started, please run 'user_scheduler <queues>' first.\n\n");
        exit(-1);
    }

    printf("Type: %d\n", scheduler_shared_memory->type);
    continue_parent_process();
    printf("Done!\n");
}

void destroy_scheduler(int signal) {
    if(scheduler.started) {
        shell_sched_dettach_shared_memory(scheduler_shared_memory);
        shell_sched_process_queue_free(scheduler.process_queue);
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
    ShellSchedMessage msg;
    msg.type = 1;
    strcpy(msg.text, "scheduler-ready");

    int msgid = shell_sched_get_msg();
    if (msgid < 0) {
        shell_sched_throw_execution_error("[ShellSchedError] Could not get message queue id.\n");
    }

    shell_sched_snd(msgid, &msg);
}
