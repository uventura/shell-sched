#include "shell_sched/core/shared.h"
#include "shell_sched/core/common.h"
#include "shell_sched/core/exceptions.h"

#include <sys/mman.h>
#include <sys/ipc.h>
#include <stdio.h>

#define SHELL_SCHED_SHARED_MEMORY_ID_DEFAULT 0x1223

key_t shell_sched_shared_memory_key;
int shell_sched_shared_memory_id;

int shell_sched_init_shared_memory() {
    shell_sched_shared_memory_key = SHELL_SCHED_SHARED_MEMORY_ID_DEFAULT;

    do {
        shell_sched_shared_memory_id = shmget(shell_sched_shared_memory_key, sizeof(ShellSchedSharedMemData), IPC_CREAT | 0666);
        shell_sched_shared_memory_key++;
    } while(shell_sched_shared_memory_id < 0);

    printf("%x\n", shell_sched_shared_memory_key);
    return shell_sched_shared_memory_id;
}

void shell_sched_destroy_shared_memory() {
    shmctl(shell_sched_shared_memory_id, IPC_RMID, NULL);
}

ShellSchedSharedMemData* shell_sched_attach_shared_memory() {
    ShellSchedSharedMemData* attach_result = shmat(shell_sched_shared_memory_id, (char*)0, 0);
    if(attach_result == (ShellSchedSharedMemData*)-1) {
        shell_sched_throw_execution_error("[ShellSchedError] The shared memory couldn't be attached.\n");
    }
    return attach_result;
}

int shell_sched_get_shared_memory() {
    return shmget(shell_sched_shared_memory_key, sizeof(ShellSchedSharedMemData), IPC_CREAT | 0x666);
}

void shell_sched_dettach_shared_memory(ShellSchedSharedMemData* memory) {
    shmdt(memory);
}

void shell_sched_write_shared_memory(ShellSchedSharedMemData* memory, ShellSchedSharedMemData data) {
    *memory = data;
}

ShellSchedSharedMemData shell_sched_read_shared_memory(ShellSchedSharedMemData* memory) {
    return *memory;
}
