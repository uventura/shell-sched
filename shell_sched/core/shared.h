#ifndef SHELL_SCHED_SHARED
#define SHELL_SCHED_SHARED

#include "shell_sched/core/common.h"
#include "shell_sched/core/process.h"

#include <sys/ipc.h>
#include <sys/shm.h>

#define SHELL_SCHED_SHARED_MEMORY_ID 0x23579
#define SHELL_SCHED_SHARED_MEMORY_SIZE 4096

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
void shell_sched_write_shared_memory(int shared_memory_id, ShellSchedSharedMemData* data);
ShellSchedSharedMemData shell_sched_read_shared_memory(int shared_memory_id);
int shell_sched_get_shared_memory();

#endif
