#include "shell_sched/exceptions.h"

#include <stdio.h>
#include <stdlib.h>

void shell_sched_check_scanf_result(int result) {
    if(result == 0) {
        printf("[ShellSchedError] Scan result was an error.\n");
        exit(-1);
    }
}

void shell_sched_throw_execution_error(const char* error_message, ...) {
    va_list args;
    va_start(args, error_message);
    vprintf(error_message, args);
    va_end(args);
    exit(SHELL_SCHED_EXIT_FAILURE);
}
