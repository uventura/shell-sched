#include "shell_sched/core/process.h"

void shell_sched_process_queue_init(ShellSchedProcessQueue* queue) {
    queue->front = NULL;
    queue->size = 0;
}

void shell_sched_process_queue_free(ShellSchedProcessQueue* queue) {
    ShellSchedProcess* process = queue->front;
    while (process != NULL)
    {
        ShellSchedProcess* next = process->next;
        free(process);
        process = next;
    }

    queue->front = NULL;
    queue->size = 0;
}

void shell_sched_process_queue_push(ShellSchedProcessQueue* queue, ShellSchedProcess* process) {
    ShellSchedProcess* current = queue->front;
    if(current == NULL) {
        queue->front = process;
        return;
    }

    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = process;
}

ShellSchedProcessQueue* shell_sched_process_queue_pop(ShellSchedProcessQueue* queue) {
    if(queue->front == NULL) {
        return NULL;
    }

    ShellSchedProcess* pop = queue->front;
    queue->front = queue->front->next;

    return pop;
}

