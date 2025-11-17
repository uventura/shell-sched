#define _POSIX_C_SOURCE 200809L

#define MAX_PROCS 256
#define DEFAULT_QUANTUM_SEC 1
#include "shell_sched/core/process.h"

SchedProcess processes[MAX_PROCS]; // tabela de processos
int proc_count = 0;

int queues_arr[3][MAX_PROCS];
int queues_head[3] = {0,0,0}; // guarda pids dos próximos a serem executados em cada fila de prioridade
int queues_tail[3] = {0,0,0}; // últimos processos já executados
int num_queues = 0;

int curr_idx = -1; // processo sendo executado
int quantum = DEFAULT_QUANTUM_SEC;
int scheduler_qid = -1;

volatile __sig_atomic_t timer_expired = 0; // boolean alterado por SIGALARM

// helpers
void print_proc_table(){
    printf("=== Scheduler State ===\n");
    for (int q=0;q<num_queues;q++) {
        printf("Queue %d:\n", q+1);
        int h = queues_head[q];
        int t = queues_tail[q];
        int len = (t - h + MAX_PROCS) % MAX_PROCS;
        if (len == 0) {
            printf("  (empty)\n");
            continue;
        }
        for (int i=0;i<len;i++) {
            int pos = (h + i) % MAX_PROCS;
            int idx = queues_arr[q][pos];
            if (idx >= 0 && processes[idx].active) {
                printf("  slot=%d pid=%d prio=%d cmd='%s' rem=%d\n", idx, processes[idx].pid, processes[idx].priority, processes[idx].command, processes[idx].remaining);
            }
        }
    }
    if (curr_idx >= 0 && processes[curr_idx].active) {
        printf("Running: slot=%d pid=%d prio=%d cmd='%s' rem=%d\n", curr_idx, processes[curr_idx].pid, processes[curr_idx].priority, processes[curr_idx].command, processes[curr_idx].remaining);
    } else {
        printf("Running: (none)\n");
    }
    printf("=======================\n");

}


 int get_free_proc_slot(){
    for (int i=0;i<MAX_PROCS;i++) {
        if (!processes[i].active) return i;
    }
    return -1;
}



int enqueue_proc_idx(int queue, int proc_idx){
    if(queue < 0 || queue > 2) { printf(">> ERRO: n fila errado\n"); return -1; }
    int t = queues_tail[queue];
    queues_arr[queue][t] = proc_idx;
    queues_tail[queue] = (t + 1) % MAX_PROCS;
    return 0;
}

 int dequeue_proc_idx(int queue){
    if(queue < 0 || queue > 2) { printf(">> ERRO: n fila errado\n"); return -1; }
    int h = queues_head[queue];
    if(h == queues_tail[queue]) {
        // fila vazia
        return -1;
    }
    int proc_idx = queues_arr[queue][h];
    queues_head[queue] = (h + 1) % MAX_PROCS;
    return proc_idx;
}

 void enqueue_front_proc_idx(int queue, int proc_idx) {
    if (queue < 0 || queue >= 3) return;
    // move head back one
    queues_head[queue] = (queues_head[queue] - 1 + MAX_PROCS) % MAX_PROCS;
    queues_arr[queue][queues_head[queue]] = proc_idx;
}

 void stop_current_process_and_enqueue(int place_at_front) {
    if (curr_idx < 0) return;
    int child = processes[curr_idx].pid;
    if (child > 0) {
        // stop child
        kill(child, SIGSTOP);
    }

    int q = processes[curr_idx].priority - 1;
    if (place_at_front) {
        enqueue_front_proc_idx(q, curr_idx);
    } else {
        enqueue_proc_idx(q, curr_idx);
    }

    printf("[Scheduler Info] Stopped pid=%d and placed in queue %d (front=%d)\n", child, q+1, place_at_front);
    curr_idx = -1;
}

void sigalarm_timer_expired(int signum){
    (void)signum;
    timer_expired = 1;
}


void handle_list_request() {
    print_proc_table();
}

void start_next_proc(){
    if (curr_idx >= 0) return; // cpu ocupada

    for (int q=0;q<num_queues;q++) {
        int idx = dequeue_proc_idx(q);
        if (idx >= 0) {
            pid_t child = fork();
            if (child < 0) {
                perror("fork");
                enqueue_front_proc_idx(q, idx);
                return;
            }
            if (child == 0) {
                execlp("/bin/sh", "sh", "-c", processes[idx].command, (char*)NULL);
                perror("execlp");
                _exit(127);
            } else {
                processes[idx].pid = child;
                processes[idx].begin = time(NULL);
                curr_idx = idx;

                kill(child, SIGCONT);

                struct itimerval tv;
                tv.it_value.tv_sec = quantum;
                tv.it_value.tv_usec = 0;
                tv.it_interval.tv_sec = 0;
                tv.it_interval.tv_usec = 0;
                setitimer(ITIMER_REAL, &tv, NULL);

                timer_expired = 0;

                int status;
                while (1) {
                    pid_t w = waitpid(child, &status, WUNTRACED | WNOHANG);
                    if (w == -1) {
                        perror("waitpid");
                        break;
                    }
                    if (w == child) {
                        if (WIFEXITED(status) || WIFSIGNALED(status)) {
                            // terminado
                            processes[idx].end = time(NULL);
                            processes[idx].active = 0;
                            printf("[Scheduler Info] Process slot=%d pid=%d finished\n", idx, child);
                            curr_idx = -1;
                            break;
                        }
                        if (WIFSTOPPED(status)) {
                            // interrompido
                            break;
                        }
                    }

                    if (timer_expired) {
                        timer_expired = 0;
                        printf("[Scheduler Info] Quantum expired for pid=%d\n", child);
                        stop_current_process_and_enqueue(0); // vai pro final
                        break;
                    }

                    // evita busy wait
                    sleep(50000);
                }

                return;
            }
        }
    }
}

// cria novo proc e bota na fila
void register_proc_from_message(const ShellSchedMsgNewProcess *msg) {
    if (!msg) return;
    if (num_queues <= 0) return;

    int q = msg->priority;
    if (q < 1 || q > num_queues) {
        printf("[ProcessError] Invalid priority %d (num queues=%d)\n", q, num_queues);
        return;
    }

    int slot = get_free_proc_slot();
    if (slot < 0) {
        printf("[ProcessError] process table full\n");
        return;
    }

    processes[slot].pid = -1;
    processes[slot].priority = q;
    processes[slot].remaining = 20; // default
    snprintf(processes[slot].command, SHELL_SCHED_CMD_MAX, "%s", msg->command);
    processes[slot].begin = 0;
    processes[slot].end = 0;
    processes[slot].active = 1;

    enqueue_proc_idx(q-1, slot);

    printf("[Scheduler Info] New process enqueued (slot=%d cmd='%s' prio=%d)\n", slot, processes[slot].command, q);

    // interrompe cpu se esse novo processo tem mais prioridade
    if (curr_idx >= 0) {
        if (processes[slot].priority < processes[curr_idx].priority) {
            printf("[Scheduler Info] Preempting current pid=%d for higher priority pid slot=%d\n", processes[curr_idx].pid, slot);
            stop_current_process_and_enqueue(1); // proc interrompido vai pra frente da fila
            start_next_proc();
        }
    } else {
        start_next_proc();
    }
}

void stop_proc_and_enqueue(int place_at_front){
    if (curr_idx < 0) return; // nada em execucao

    pid_t child = processes[curr_idx].pid;

    kill(child, SIGSTOP);

    // update remaining time
    processes[curr_idx].remaining -= quantum;
    if (processes[curr_idx].remaining < 0) processes[curr_idx].remaining = 0;

    // volta pra fila
    int prio = processes[curr_idx].priority;
    if (place_at_front) {
        int h = queues_head[prio - 1];
        queues_head[prio - 1] = (h - 1 + MAX_PROCS) % MAX_PROCS;
        queues_arr[prio - 1][queues_head[prio - 1]] = curr_idx;
    } else {
        enqueue_proc_idx(prio - 1, curr_idx);
    }

    printf("[Scheduler Info] Process slot=%d pid=%d preempted and re-enqueued\n", curr_idx, child);

    curr_idx = -1;
}

int remove_proc_from_queue(int proc_idx) {
    for (int q=0;q<num_queues;q++) {
        int h = queues_head[q];
        int t = queues_tail[q];
        int len = (t - h + MAX_PROCS) % MAX_PROCS;
        for (int i=0;i<len;i++) {
            int pos = (h + i) % MAX_PROCS;
            if (queues_arr[q][pos] == proc_idx) {
                // shift following
                for (int j=i;j<len-1;j++) {
                    int from = (h + j + 1) % MAX_PROCS;
                    int to = (h + j) % MAX_PROCS;
                    queues_arr[q][to] = queues_arr[q][from];
                }
                queues_tail[q] = (queues_tail[q] - 1 + MAX_PROCS) % MAX_PROCS;
                return 0;
            }
        }
    }
    return -1;
}

void handle_exit_request() {
    printf("[Scheduler] Exit requested. Cleaning up...\n");

    struct itimerval z = {0};
    setitimer(ITIMER_REAL, &z, NULL);

    // termina
    if (curr_idx >= 0) {
        int pid = processes[curr_idx].pid;
        if (pid > 0) kill(pid, SIGTERM);
    }

    // termina filas 
    for (int i=0;i<MAX_PROCS;i++) {
        if (processes[i].active) {
            if (processes[i].pid > 0) {
                // espera child terminar
                int status;
                waitpid(processes[i].pid, &status, 0);
                processes[i].end = time(NULL);
                processes[i].active = 0;
            }
        }
    }

    // print turnaround 
    printf("--- Turnaround times ---\n");
    for (int i=0;i<MAX_PROCS;i++) {
        if (processes[i].begin > 0) {
            time_t endt = processes[i].end > 0 ? processes[i].end : time(NULL);
            printf("slot=%d cmd='%s' start=%ld end=%ld turnaround=%ld\n", i, processes[i].command, (long)processes[i].begin, (long)endt, (long)(endt - processes[i].begin));
        }
    }

    // remove message queue
    if (scheduler_qid >= 0) {
        msgctl(scheduler_qid, IPC_RMID, NULL);
    }

    printf("[Scheduler] Exiting now.\n");
    _exit(0);
}

// escalonador, qid vem do parent
void run_scheduler_loop(int qid, int queues) {
    // basic init
    num_queues = queues;
    scheduler_qid = qid;

    // inicializa filas, tabela de processos
    for (int i=0;i<MAX_PROCS;i++) processes[i].active = 0;
    for (int q=0;q<3;q++) {
        queues_head[q] = queues_tail[q] = 0;
    }

    struct sigaction sa;
    sa.sa_handler = sigalarm_timer_expired;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    printf("[Scheduler] Running (queues=%d qid=%d)\n", num_queues, qid);


    ShellSchedMessage msg;
    size_t msgsz = sizeof(ShellSchedMsgNewProcess) - sizeof(long);

    while (1) {
        ssize_t r = msgrcv(qid, &msg, msgsz, 0, 0);
        if (r < 0) {
            if (errno == EINTR) continue; // interrompido
            perror("msgrcv");
            break;
        }

        switch (msg.mtype) {
            case SHELL_SCHED_MSG_NEW_PROCESS:
                register_proc_from_message(&msg.newProcess);
                break;
            case SHELL_SCHED_MSG_LIST_REQUEST:
                handle_list_request();
                break;
            case SHELL_SCHED_MSG_EXIT_REQUEST:
                handle_exit_request();
                break;
            default:
                printf("[Scheduler] unknown message type %ld\n", msg.mtype);
                break;
        }

        // executa prox proc at idle
        if (curr_idx < 0) {
            start_next_proc();
        }
    }

    // cleanup
    handle_exit_request();
}

