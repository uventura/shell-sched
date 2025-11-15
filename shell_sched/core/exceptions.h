#ifndef SHELL_SCHED_EXCEPTIONS_H
#define SHELL_SCHED_EXCEPTIONS_H

#include "shell_sched/core/common.h"

#define SHELL_SCHED_EXIT_FAILURE 1

void shell_sched_check_scanf_result(int result);
void shell_sched_throw_execution_error(const char* error_message, ...);

#endif
