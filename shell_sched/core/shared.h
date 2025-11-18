#ifndef SHELL_SCHED_SHARED
#define SHELL_SCHED_SHARED

#include "shell_sched/core/common.h"
#include "shell_sched/core/process.h"

#include <sys/ipc.h>
#include <sys/shm.h>

typedef enum {
    INT_SHARED,
    NEW_PROCESS_SHARED
} ShellSchedSharedMemType;

typedef struct {
    ShellSchedSharedMemType type;
    union {
        int i32;
        ShellSchedNewProcess process;
    };
} ShellSchedSharedMemData;

int shell_sched_init_shared_memory();
void shell_sched_destroy_shared_memory();

ShellSchedSharedMemData* shell_sched_attach_shared_memory();
void shell_sched_dettach_shared_memory(ShellSchedSharedMemData* memory);

void shell_sched_write_shared_memory(ShellSchedSharedMemData* memory, ShellSchedSharedMemData data);
ShellSchedSharedMemData shell_sched_read_shared_memory(ShellSchedSharedMemData* memory);

#endif
