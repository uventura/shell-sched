#include "shell_sched/exceptions.h"

#include <stdio.h>
#include <stdlib.h>

#define SHELL_SCHED_EXIT_FAILURE 1

void shell_sched_throw_execution_error(const char* error_message, ...) {
    va_list args;
    va_start(args, error_message);
    vprintf(error_message, args);
    va_end(args);
    exit(SHELL_SCHED_EXIT_FAILURE);
}
