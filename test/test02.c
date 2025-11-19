#include <stdio.h>
#include <unistd.h>

int main() {
    setbuf(stdout, NULL);
    printf("Job started (PID: %d)\n", getpid());
    for (int i = 0; i < 20; i++) {
        printf("Working... %d/20\n", i + 1);
        fflush(stdout);
        sleep(5);
    }
    printf("Job completed 'small_for'\n");
    fflush(stdout);
    return 0;
}
