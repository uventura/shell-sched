#include "shell_sched/core/run.h"
#include "shell_sched/core/common.h"
#include "shell_sched/core/scheduler.h"
#include "shell_sched/core/process.h"
#include "shell_sched/core/exceptions.h"
#include "shell_sched/core/shared.h"

#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>

#define _POSIX_C_SOURCE 200809L
#define __USE_POSIX 200809L
#include <signal.h>

#define RUNNING 1
#define MAX_COMMAND_SIZE 1000
#define STR_EQUAL 0
#define CHILD_PROCESS 0

int run_share_memory_id;
ShellSchedSharedMemData* run_shared_memory;

pid_t scheduler_pid;
bool scheduler_started = false;

void user_scheduler(void);
void execute_process(void);
void list_scheduler(void);
void exit_scheduler(void);
void help_scheduler(void);

void continue_after_scheduler_signal(int signal);
void wait_scheduler_finish_action(void);

void shell_sched_run() {
    scheduler_started = false;
    signal(SIGCONT, continue_after_scheduler_signal);

    shell_sched_init_shared_memory();
    run_shared_memory = shell_sched_attach_shared_memory();


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
    int scheduler_queues;
    shell_sched_check_scanf_result(scanf("%d", &scheduler_queues));

    ShellSchedSharedMemData data;
    data.type = INT_SHARED;
    data.i32 = scheduler_queues;

    shell_sched_write_shared_memory(run_shared_memory, data);

    scheduler_pid = fork();
    if(scheduler_pid < 0) {
        printf("[ShellSchedError] The user scheduler couldn't be started.");
    } else if(scheduler_pid == CHILD_PROCESS) {
        shell_sched_init_scheduler();
        shell_sched_run_scheduler();
    } else {
        scheduler_started = true;
    }

    wait_scheduler_finish_action();
    printf("\n");
}

void execute_process(void) {
    ShellSchedNewProcess process;
    shell_sched_check_scanf_result(scanf("%s %d", process.command, &process.priority));

    ShellSchedSharedMemData data;
    data.type = NEW_PROCESS_SHARED;
    data.process = process;
    shell_sched_write_shared_memory(run_shared_memory, data);

    
    wait_scheduler_finish_action();
}

void list_scheduler(void) {
    // TODO
}

void exit_scheduler(void) {
    if(scheduler_started) {
        kill(scheduler_pid, SIGQUIT);
        wait(NULL);
    }
    shell_sched_destroy_shared_memory();
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

void continue_after_scheduler_signal(int signal) {}

void wait_scheduler_finish_action(void) {
    // Wait a signal which means that the process has been registered.
    pause();
}
