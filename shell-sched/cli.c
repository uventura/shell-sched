#include "shell-sched/cli.h"
#include "shell-sched/common.h"

ShellSchedCli auri_cli(int argc, char* argv[]) {
    ShellSchedCli cli;
    cli.help = false;

    for(int i=1; i < argc; ++i) {
        if(strcmp(argv[i], "--help") == 0) {
            shell_sched_help();
            break;
        }
    }

    return cli;
}

void shell_sched_help(void) {
    printf("====================================\n");
    printf("|               HELP               |\n");
    printf("====================================\n");

    // TODO

    printf("====================================\n");
}
