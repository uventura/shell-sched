#ifndef SHELL_SCHED_SHARED
#define SHELL_SCHED_SHARED

#include "shell_sched/core/common.h"

#include <sys/ipc.h>
#include <sys/shm.h>

#define SHELL_SCHED_SHARED_MEMORY_ID 0x23579
#define SHELL_SCHED_SHARED_MEMORY_SIZE 4096

int shell_sched_init_shared_memory();
int shell_sched_get_shared_memory();

#endif
