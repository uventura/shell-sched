#ifndef SHELL_SCHED_SCHEDULE_H
#define SHELL_SCHED_SCHEDULE_H

#include "shell_sched/core/common.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SCHEDULER_DEFAULT_ID 0X1234
#define SCHEDULER_DEFAULT_FLAGS (IPC_CREAT | 0x666)

typedef struct {
    bool started;
    int id;
    key_t key;
    int flags;
    int queues;
} ShellSchedScheduler;

#endif
