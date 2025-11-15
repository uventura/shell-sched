#include "shell_sched/run.h"
#include "shell_sched/common.h"

#include <stdio.h>

#define RUNNING 1
#define MAX_COMMAND_SIZE 1000
#define STR_EQUAL 0

void user_scheduler(void);
void execute_process(void);
void list_scheduler(void);
void exit_scheduler(void);

void shell_sched_run() {
    while(RUNNING) {
        printf("> shell_sched: ");

        char command[MAX_COMMAND_SIZE];
        scanf("%s", command);

        if(strcmp(command, "user_scheduler") == STR_EQUAL) {
            user_scheduler();
        } else if(strcmp(command, "execute_process") == STR_EQUAL) {
            execute_process();
        }else if(strcmp(command, "list_scheduler") == STR_EQUAL) {
            list_scheduler();
        } else if(strcmp(command, "exit_scheduler") == STR_EQUAL) {
            exit_scheduler();
            break;
        }
    }
}

void user_scheduler(void) {
    // TODO
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
