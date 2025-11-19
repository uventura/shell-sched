#include "shell_sched/core/process.h"

#include <malloc.h>

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
        queue->size += 1;
        queue->front = process;
        return;
    }

    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = process;
    queue->size += 1;
}

ShellSchedProcess* shell_sched_process_queue_pop(ShellSchedProcessQueue* queue) {
    if(queue->front == NULL) {
        return NULL;
    }

    ShellSchedProcess* pop = queue->front;
    queue->front = queue->front->next;
    queue->size -= 1;

    pop->next = NULL;
    return pop;
}

