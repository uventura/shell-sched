// gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)
// ALUNOS:
// Aquila Macedo Costa          202021800
// Carolina Fernandes de Campos 221030830
// Regina Emy                   190037351
// Ualiton Ventura da Silva     202033580

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

typedef struct {
    long type;
    char text[128];
} ShellSchedMessage;

int shell_sched_init_msg();
void shell_sched_destroy_msg();
int shell_sched_get_msg();
void shell_sched_snd(int id, ShellSchedMessage* msg);
void shell_sched_rcv(int id, ShellSchedMessage* msg);

int shell_sched_init_shared_memory();
void shell_sched_destroy_shared_memory();
int shell_sched_get_shared_memory();

ShellSchedSharedMemData* shell_sched_attach_shared_memory();
void shell_sched_dettach_shared_memory(ShellSchedSharedMemData* memory);

void shell_sched_write_shared_memory(ShellSchedSharedMemData* memory, ShellSchedSharedMemData data);
ShellSchedSharedMemData shell_sched_read_shared_memory(ShellSchedSharedMemData* memory);

#endif
