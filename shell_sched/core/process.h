// gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)
// ALUNOS:
// Aquila Macedo Costa          202021800
// Carolina Fernandes de Campos 221030830
// Regina Emy                   190037351
// Ualiton Ventura da Silva     202033580

#ifndef SHELL_SCHED_PROCESS_H
#define SHELL_SCHED_PROCESS_H

#include "shell_sched/core/common.h"

#include <sys/types.h>

#define SHELL_SCHED_CMD_MAX 4096

typedef struct {
    int priority;
    char command[SHELL_SCHED_CMD_MAX];
} ShellSchedNewProcess;

typedef struct SchedProcess {
    int pid;
    int remaining;
    int priority;
    int output_pipe[2];
    struct SchedProcess* next;
} ShellSchedProcess;

typedef struct {
    int size;
    ShellSchedProcess* front;
} ShellSchedProcessQueue;

void shell_sched_process_queue_init(ShellSchedProcessQueue* queue);
void shell_sched_process_queue_free(ShellSchedProcessQueue* queue);
void shell_sched_process_queue_push(ShellSchedProcessQueue* queue, ShellSchedProcess* process);
ShellSchedProcess* shell_sched_process_queue_pop(ShellSchedProcessQueue* queue);

#endif
