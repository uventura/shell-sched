#ifndef SHELL_SCHED_PROCESS_H
#define SHELL_SCHED_PROCESS_H

#include "shell_sched/core/common.h"

#include <sys/types.h>
#include <sys/ipc.h>

#define SHELL_SCHED_CMD_MAX 256

typedef enum {
    SHELL_SCHED_MSG_NEW_PROCESS = 1,
    SHELL_SCHED_MSG_LIST_REQUEST = 2,
    SHELL_SCHED_MSG_EXIT_REQUEST = 3
} ShellSchedMsgType;

typedef struct {
    int pid;
    int remaining;
} ShellSchedProcess;

typedef struct {
    long mtype; // must be first for System V IPC
    int priority; // 1 is highest
    char command[SHELL_SCHED_CMD_MAX];
} ShellSchedMsgNewProcess;

typedef union {
    long mtype;
    ShellSchedMsgNewProcess newProcess;
} ShellSchedMessage;

// adicione em process.h (no final do arquivo, antes do #endif)
void shell_sched_scheduler_main(key_t key, int flags, int queues);


#endif
