#include <stdio.h>
#include <unistd.h>

int main() {
    setbuf(stdout, NULL);
    for (int i = 0; i < 40; i++) {
        sleep(10);
    }
    fflush(stdout);
    return 0;
}
