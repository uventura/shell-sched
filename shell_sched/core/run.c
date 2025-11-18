#include "shell_sched/core/run.h"
#include "shell_sched/core/common.h"
#include "shell_sched/core/scheduler.h"
#include "shell_sched/core/process.h"
#include "shell_sched/core/exceptions.h"
#include "shell_sched/core/shared.h"

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define RUNNING 1
#define MAX_COMMAND_SIZE 1000
#define STR_EQUAL 0
#define CHILD_PROCESS 0

bool scheduler_started = false;

void user_scheduler(void);
void execute_process(void);
void list_scheduler(void);
void exit_scheduler(void);
void help_scheduler(void);

void shell_sched_run() {
    scheduler_started = false;
    shell_sched_init_shared_space();

    while(RUNNING) {
        printf("> shell_sched: ");

        char command[MAX_COMMAND_SIZE];

        int scan_result = scanf("%s", command);
        shell_sched_check_scanf_result(scan_result);

        if(strcmp(command, "user_scheduler") == STR_EQUAL) {
            user_scheduler();
        } else if(strcmp(command, "execute_process") == STR_EQUAL) {
            execute_process();
        }else if(strcmp(command, "list_scheduler") == STR_EQUAL) {
            list_scheduler();
        } else if(strcmp(command, "exit_scheduler") == STR_EQUAL) {
            exit_scheduler();
            break;
        } else if(strcmp(command, "help") == STR_EQUAL) {
            help_scheduler();
        } else {
            printf("[ShellSchedError] The command could not be found.\nType 'help' to get the available commands.\n\n");
        }
    }
}

void user_scheduler(void) {
    pid_t pid = fork();

    if(pid < 0) {
        printf("[ShellSchedError] The user scheduler couldn't be started.");
    } else if(pid == CHILD_PROCESS) {
        shell_sched_init_scheduler();
        shell_sched_run_scheduler();
    } else {
        scheduler_started = true;
    }
}

void execute_process(void) {

}

void list_scheduler(void) {
    // TODO
}

void exit_scheduler(void) {
    if(scheduler_started) {

    }
    printf("Bye! :)\n");
}

void help_scheduler(void) {
    printf("====================================\n");
    printf("|               HELP               |\n");
    printf("====================================\n");
    printf("Available Commands:\n");
    printf("| user_scheduler <Number of Queues>       | Create queues\n");
    printf("| execute_process <Command> <Priority>    | Enqueue a process request\n");
    printf("| list_scheduler                          | List available schedulings\n");
    printf("| exit_scheduler                          | Exit scheduler\n");
    printf("| help                                    | To get available commands.\n");
    printf("====================================\n");
}
