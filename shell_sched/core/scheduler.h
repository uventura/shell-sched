// gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)
// ALUNOS:
// Aquila Macedo Costa          202021800
// Carolina Fernandes de Campos 221030830
// Regina Emy                   190037351
// Ualiton Ventura da Silva     202033580

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
    ShellSchedProcess* running_process;
    ShellSchedProcessQueue process_queue[SCHEDULER_MAX_QUEUES];
} ShellSchedScheduler;

void shell_sched_init_scheduler();
void shell_sched_run_scheduler();

#endif
