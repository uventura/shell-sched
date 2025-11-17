#ifndef SHELL_SCHED_PROCESS_H
#define SHELL_SCHED_PROCESS_H

#define SHELL_SCHED_CMD_MAX 256

#include "shell_sched/core/common.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>


typedef enum {
    SHELL_SCHED_MSG_NEW_PROCESS = 1,
    SHELL_SCHED_MSG_LIST_REQUEST = 2,
    SHELL_SCHED_MSG_EXIT_REQUEST = 3
} ShellSchedMsgType;

typedef struct {
    int pid;
    int priority; // 1 (highest) .. 3 item 3.2
    int remaining; // tempo de execução restante
    char command[SHELL_SCHED_CMD_MAX];
    time_t begin;
    time_t end;
    int active; // true quando na table
} SchedProcess;

typedef struct {
    long mtype; // must be first for System V IPC
    int priority; // 1 is highest
    char command[SHELL_SCHED_CMD_MAX];
} ShellSchedMsgNewProcess;

typedef union {
    long mtype;
    ShellSchedMsgNewProcess newProcess;
} ShellSchedMessage;



void register_proc_from_message(const ShellSchedMsgNewProcess *msg);
void start_next_proc();
void stop_proc_and_enqueue(int place_at_front);
int remove_proc_from_queue(int proc_idx);
void handle_exit_request() ;
void sigalarm_timer_expired(int signum);
void run_scheduler_loop(int qid, int queues);
void handle_list_request();

// helpers
void print_proc_table(); 
int get_free_proc_slot();
int enqueue_proc_idx(int queue, int proc_idx); 
int dequeue_proc_idx(int queue);
void enqueue_front_proc_idx(int queue, int proc_idx) ;
void stop_current_process_and_enqueue(int place_at_front) ;
void sigalarm_timer_expired(int signum);


#endif
