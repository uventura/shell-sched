#ifndef SHELL_SCHED_SCHEDULE_H
#define SHELL_SCHED_SCHEDULE_H

#include "shell_sched/core/common.h"
#include "shell_sched/core/process.h"

#include <sys/ipc.h>
#include <sys/msg.h>

#define SCHEDULER_DEFAULT_ID 0X1234
#define SCHEDULER_DEFAULT_FLAGS (IPC_CREAT | 0666)
#define SCHEDULER_MAX_QUEUES 3

typedef struct {
    bool started;
    pid_t parent;
    int queues;
    ShellSchedProcessQueue process_queue[SCHEDULER_MAX_QUEUES];
} ShellSchedScheduler;

void shell_sched_init_scheduler();
void shell_sched_run_scheduler();

#endif
