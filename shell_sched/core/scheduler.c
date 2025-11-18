#include "shell_sched/core/scheduler.h"
#include "shell_sched/core/common.h"
#include "shell_sched/core/process.h"
#include "shell_sched/core/exceptions.h"

#include <stdio.h>
#include <errno.h>

ShellSchedScheduler scheduler;

void execute_process_scheduler();

void shell_sched_init_scheduler() {
    shell_sched_check_scanf_result(scanf("%d", &scheduler.queues));
    if(scheduler.queues <= 0 || scheduler.queues > 3) {
        shell_sched_throw_execution_error("[ShellSchedError] The number of queues can be 2 or 3.\n");
        return;
    }

    scheduler.key = SCHEDULER_DEFAULT_ID;
    scheduler.flags = SCHEDULER_DEFAULT_FLAGS;
    scheduler.id = msgget(scheduler.key, scheduler.flags);

    if(scheduler.id < 0) {
        int saved_errno = errno;
        perror("[ShellSchedError] msgget");

        // fallback, create a private queue to bypass bad-perm queues
        if(saved_errno == EACCES) {
            scheduler.id = msgget(IPC_PRIVATE, SCHEDULER_DEFAULT_FLAGS);
            if(scheduler.id < 0) {
                perror("[ShellSchedError] msgget(IPC_PRIVATE)");
                shell_sched_throw_execution_error("[ShellSchedError] The scheduler queue couldn't be created.\n\n");
            }
            printf("[Warn] Using a private message queue (id=%d) due to permission issues on the default key.\n", scheduler.id);
        } else {
            shell_sched_throw_execution_error("[ShellSchedError] The scheduler queue couldn't be created.\n\n");
            return;
        }
    }

    scheduler.started = true;
    printf("Scheduler queue created.\n\n");
}

void shell_sched_run_scheduler() {

}

void shell_sched_destroy_scheduler() {
    if(scheduler.started) {
        struct msqid_ds removed_queue_result;
        int remove_result = msgctl(scheduler.id, IPC_RMID, &removed_queue_result);

        if(remove_result == -1) {
            shell_sched_throw_execution_error("[ShellSchedError] The queue couldn't be removed.\n");
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
}


void execute_process_scheduler() {
    if(!scheduler.started) {
        shell_sched_throw_execution_error("[ShellSchedError] The scheduler is not started, please run 'user_scheduler <queues>' first.\n\n");
        exit(-1);
    }

    ShellSchedMsgNewProcess msg;
    msg.mtype = SHELL_SCHED_MSG_NEW_PROCESS;

    // read command (single token) and priority directly into the message
    shell_sched_check_scanf_result(scanf("%s %d", msg.command, &msg.priority));

    if(msg.priority < 1 || msg.priority > scheduler.queues) {
        shell_sched_throw_execution_error("[ShellSchedError] Invalid priority. It must be between 1 and %d.\n\n", scheduler.queues);
        return;
    }

    size_t msgsz = sizeof(ShellSchedMsgNewProcess) - sizeof(long);
    int send_result = msgsnd(scheduler.id, &msg, msgsz, 0);
    if(send_result == -1) {
        perror("[ShellSchedError] msgsnd");
        shell_sched_throw_execution_error("[ShellSchedError] Failed to enqueue process request (qid=%d).\n\n", scheduler.id);
        return;
    }

    printf("[Info] Process request submitted: command='%s', priority=%d.\n\n", msg.command, msg.priority);
}
