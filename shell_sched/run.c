#include "shell_sched/run.h"
#include "shell_sched/common.h"
#include "shell_sched/scheduler.h"

#include <stdlib.h>
#include <stdio.h>

#define RUNNING 1
#define MAX_COMMAND_SIZE 1000
#define STR_EQUAL 0

ShellSchedScheduler scheduler;

void user_scheduler(void);
void execute_process(void);
void list_scheduler(void);
void exit_scheduler(void);
void help_scheduler(void);

void shell_sched_run() {
    while(RUNNING) {
        printf("> shell_sched: ");

        char command[MAX_COMMAND_SIZE];
        int scan_result = scanf("%s", command);

        if(scan_result == 0) {
            printf("[ShellSchedCliError] Scan result was an error.\n");
            exit(-1);
        }

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
            printf("[ShellSchedCliError] The command could not be found.\nType 'help' to get the available commands.\n");
        }
    }
}

void user_scheduler(void) {
    // scheduler.key = 0X1234;
    // scheduler.flags = IPC_CREAT | 0x666;
    // scheduler.id = msgget(scheduler.key, scheduler.flags);
    scanf("%d", &scheduler.queues);
    printf("Queues: %d\n", scheduler.queues);
}

void execute_process(void) {
    // TODO
}

void list_scheduler(void) {
    // TODO
}

void exit_scheduler(void) {
    printf("Bye! :)\n");
}

void help_scheduler(void) {
    printf("====================================\n");
    printf("|               HELP               |\n");
    printf("====================================\n");
    printf("Available Commands:\n");
    printf("| user_scheduler <Number of Queues>     | Create queues\n");
    printf("| execute_scheduler <Command Priority>  | Execute scheduler\n");
    printf("| list_scheduler                        | List available schedulings\n");
    printf("| exit_scheduler                        | Exit scheduler\n");
    printf("| help                                  | To get available commands.\n");
    printf("====================================\n");
}
