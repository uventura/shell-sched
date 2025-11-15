#ifndef SHELL_SCHED_EXCEPTIONS_H
#define SHELL_SCHED_EXCEPTIONS_H

#include "shell_sched/common.h"

void shell_sched_throw_execution_error(const char* error_message, ...);

#endif
