#include "shell_sched/core/run.h"
#include "shell_sched/core/common.h"
#include "shell_sched/core/scheduler.h"
#include "shell_sched/core/process.h"
#include "shell_sched/core/exceptions.h"
#include "shell_sched/core/process.h"


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define RUNNING 1
#define MAX_COMMAND_SIZE 1000
#define STR_EQUAL 0

// QUESTIONS: São vários schedulers ou apenas um?
ShellSchedScheduler scheduler;

void create_user_scheduler(void);
void execute_process(void);
void list_scheduler(void);
void exit_scheduler(void);
void help_scheduler(void);

void shell_sched_run() {
    scheduler.started = false;

    while(RUNNING) {
        printf("> shell_sched: ");

        char command[MAX_COMMAND_SIZE];

        int scan_result = scanf("%s", command);
        shell_sched_check_scanf_result(scan_result);

        if(strcmp(command, "create_user_scheduler") == STR_EQUAL) {
            create_user_scheduler();
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

void create_user_scheduler(void) {
    shell_sched_check_scanf_result(scanf("%d", &scheduler.queues));
    if(scheduler.queues <= 0 || scheduler.queues > 3) {
        printf("[ShellSchedError] The number of queues can be 2 or 3.\n");
        return;
    }

    scheduler.key = SCHEDULER_DEFAULT_ID;
    scheduler.flags = SCHEDULER_DEFAULT_FLAGS;
    scheduler.id = msgget(scheduler.key, scheduler.flags);

    scheduler.started = true;
    printf("Scheduler queue created.\n\n");

    pid_t sched_pid = fork();
    if (sched_pid < 0) {
        perror("[ShellSchedError] fork scheduler");
        return;
    }
    if (sched_pid == 0) {
        // filho -> roda o loop do escalonador
        shell_sched_scheduler_main(scheduler.key, scheduler.flags, scheduler.queues);
        // caso a função retorne, saia
        _exit(0);
    } else {
        printf("[Info] Scheduler process forked (pid=%d)\n\n", sched_pid);
    }

    if(scheduler.id < 0) {
        int saved_errno = errno;
        perror("[ShellSchedError] msgget");
        // fallback, create a private queue to bypass bad-perm queues
        if(saved_errno == EACCES) {
            scheduler.id = msgget(IPC_PRIVATE, SCHEDULER_DEFAULT_FLAGS);
            if(scheduler.id < 0) {
                perror("[ShellSchedError] msgget(IPC_PRIVATE)");
                printf("[ShellSchedError] The scheduler queue couldn't be created.\n\n");
                return;
            }
            printf("[Warn] Using a private message queue (id=%d) due to permission issues on the default key.\n", scheduler.id);
        } else {
            printf("[ShellSchedError] The scheduler queue couldn't be created.\n\n");
            return;
        }
    }

    scheduler.started = true;
    printf("Scheduler queue created.\n\n");
}

void execute_process(void) {
    if(!scheduler.started) {
        printf("[ShellSchedError] The scheduler is not started, please run 'create_user_scheduler <queues>' first.\n\n");
        return;
    }

    // build and send a NEW_PROCESS message to the scheduler queue
    ShellSchedMsgNewProcess msg;
    msg.mtype = SHELL_SCHED_MSG_NEW_PROCESS;

    // read command (single token) and priority directly into the message
    shell_sched_check_scanf_result(scanf("%s %d", msg.command, &msg.priority));

    if(msg.priority < 1 || msg.priority > scheduler.queues) {
        printf("[ShellSchedError] Invalid priority. It must be between 1 and %d.\n\n", scheduler.queues);
        return;
    }

    size_t msgsz = sizeof(ShellSchedMsgNewProcess) - sizeof(long);
    int send_result = msgsnd(scheduler.id, &msg, msgsz, 0);
    if(send_result == -1) {
        perror("[ShellSchedError] msgsnd");
        printf("[ShellSchedError] Failed to enqueue process request (qid=%d).\n\n", scheduler.id);
        return;
    }

    printf("[Info] Process request submitted: command='%s', priority=%d.\n\n", msg.command, msg.priority);
}

void list_scheduler(void) {
    // TODO
}

void exit_scheduler(void) {
    if(scheduler.started) {
        struct msqid_ds removed_queue_result;
        int remove_result = msgctl(scheduler.id, IPC_RMID, &removed_queue_result);

        if(remove_result == -1) {
            printf("[ShellSchedError] The queue couldn't be removed.\n");
            exit(SHELL_SCHED_EXIT_FAILURE);
        }

        printf("[Scheduler Info]\n");
        printf("| msg_stime = %ld\n", removed_queue_result.msg_stime);
        printf("| msg_rtime = %ld\n", removed_queue_result.msg_rtime);
        printf("| msg_ctime = %ld\n", removed_queue_result.msg_ctime);
        printf("| msg_qnum = %ld\n", removed_queue_result.msg_qnum);
        printf("| msg_qbytes = %ld\n", removed_queue_result.msg_qbytes);
        printf("| msg_lspid = %d\n", removed_queue_result.msg_lspid);
        printf("| msg_lrpid = %d\n\n", removed_queue_result.msg_lrpid);
    }

    printf("Bye! :)\n");
}

void help_scheduler(void) {
    printf("====================================\n");
    printf("|               HELP               |\n");
    printf("====================================\n");
    printf("Available Commands:\n");
    printf("| create_user_scheduler <Number of Queues>       | Create queues\n");
    printf("| execute_process <Command> <Priority>    | Enqueue a process request\n");
    printf("| list_scheduler                          | List available schedulings\n");
    printf("| exit_scheduler                          | Exit scheduler\n");
    printf("| help                                    | To get available commands.\n");
    printf("====================================\n");
}
