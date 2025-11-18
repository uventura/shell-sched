#include "shell_sched/core/shared.h"
#include "shell_sched/core/common.h"
#include "shell_sched/core/exceptions.h"

#include <sys/ipc.h>

int shell_sched_init_shared_memory() {
    int shmid = shmget(SHELL_SCHED_SHARED_MEMORY_ID, SHELL_SCHED_SHARED_MEMORY_SIZE, IPC_CREAT | 0x666);

    if(shmid < 0) {
        shell_sched_throw_execution_error("[ShellSchedError] The shared memory couldn't be started");
    }
    return shmid;
}

int shell_sched_get_shared_memory() {
    int shmid = shmget(SHELL_SCHED_SHARED_MEMORY_ID, 0, 0x666);

    if(shmid < 0) {
        shell_sched_throw_execution_error("[ShellSchedError] The shared memory couldn't be started");
    }
    return shmid;
}
