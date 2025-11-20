// gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)
// ALUNOS:
// Aquila Macedo Costa          202021800
// Carolina Fernandes de Campos 221030830
// Regina Emy                   190037351
// Ualiton Ventura da Silva     202033580

#ifndef SHELL_SCHED_EXCEPTIONS_H
#define SHELL_SCHED_EXCEPTIONS_H

#include "shell_sched/core/common.h"

#define SHELL_SCHED_EXIT_FAILURE 1

void shell_sched_check_scanf_result(int result);
void shell_sched_throw_execution_error(const char* error_message, ...);

#endif
