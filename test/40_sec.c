#include <stdio.h>
#include <unistd.h>

int main() {
    setbuf(stdout, NULL); // pra printar a execução na ordem certa e permitir novo processo
    printf("Job started (PID: %d)\n", getpid());
    for (int i = 0; i < 40; i++) {
        printf("Working... %d/20\n", i + 1);
        fflush(stdout);
        sleep(1);
    }
    printf("Job completed\n");
    fflush(stdout);
    return 0;
}