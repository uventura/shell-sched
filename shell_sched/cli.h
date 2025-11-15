#ifndef SHELL_SCHED_CLI_H
#define SHELL_SCHED_CLI_H

#include "shell_sched/common.h"

typedef struct {
    bool help;
} ShellSchedCli;

ShellSchedCli auri_cli(int argc, char* argv[]);
void shell_sched_help(void);

#endif
