#include "shell_sched/core/shared.h"
#include "shell_sched/core/common.h"
#include "shell_sched/core/exceptions.h"

#include <sys/mman.h>
#include <sys/ipc.h>

int shell_sched_init_shared_memory() {
    int shmid = shmget(SHELL_SCHED_SHARED_MEMORY_ID, SHELL_SCHED_SHARED_MEMORY_SIZE, IPC_CREAT | 0x666);

    if(shmid < 0) {
        shell_sched_throw_execution_error("[ShellSchedError] The shared memory couldn't be started");
    }
    return shmid;
}

void shell_sched_write_shared_memory(int shared_memory_id, ShellSchedSharedMemData* data) {
    ShellSchedSharedMemData* shared_ptr = (ShellSchedSharedMemData*)shmat(shared_memory_id, NULL, 0);
    *shared_ptr = *((ShellSchedSharedMemData*)data);
}

ShellSchedSharedMemData shell_sched_read_shared_memory(int shared_memory_id) {
    ShellSchedSharedMemData* shared_ptr = (ShellSchedSharedMemData*)shmat(shared_memory_id, NULL, 0);
    return *shared_ptr;
}

int shell_sched_get_shared_memory() {
    int shmid = shmget(SHELL_SCHED_SHARED_MEMORY_ID, 0, 0x666);

    if(shmid < 0) {
        shell_sched_throw_execution_error("[ShellSchedError] The shared memory couldn't be started");
    }
    return shmid;
}
