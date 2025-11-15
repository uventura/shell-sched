#include "shell_sched/run.h"
#include "shell_sched/common.h"

#define RUNNING 1

void shell_sched_run() {
    while(RUNNING) {
        printf("> shell_sched: ");

    }
}
