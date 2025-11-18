#ifndef SHELL_SCHED_PROCESS_H
#define SHELL_SCHED_PROCESS_H

#include "shell_sched/core/common.h"

#include <sys/types.h>

#define SHELL_SCHED_CMD_MAX 4096

// typedef struct {
//     int pid;
//     int remaining;
// } ShellSchedProcess;

typedef struct {
    int priority;
    char command[SHELL_SCHED_CMD_MAX];
} ShellSchedNewProcess;

typedef struct {
    int pid;
    int remaining;
    int priority;
} ShellSchedProcess;

#endif
