#ifndef SHELL_SCHED_SCHEDULE_H
#define SHELL_SCHED_SCHEDULE_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

typedef struct {
    int id;
    key_t key;
    int flags;
    int queues;
} ShellSchedScheduler;

#endif
