// gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)
// ALUNOS:
// Aquila Macedo Costa          202021800
// Carolina Fernandes de Campos 221030830
// Regina Emy                   190037351
// Ualiton Ventura da Silva     202033580

#include "shell_sched/core/exceptions.h"

#include <stdio.h>
#include <stdlib.h>

void shell_sched_check_scanf_result(int result) {
    if(result == 0) {
        printf("[ShellSchedError] Scan result was an error.\n");
        exit(-1);
    }
}

void shell_sched_throw_execution_error(const char* error_message, ...) {
    perror("Shared memory failed");

    va_list args;
    va_start(args, error_message);
    vprintf(error_message, args);
    va_end(args);
    exit(SHELL_SCHED_EXIT_FAILURE);
}
