#include <stdio.h>
#include <unistd.h>

int main() {
    setbuf(stdout, NULL);
    for (int i = 0; i < 20; i++) {;
        sleep(5);
    }
    fflush(stdout);
    return 0;
}
