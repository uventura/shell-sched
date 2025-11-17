#include<stdio.h>
#include<unistd.h>
// para teste
int main (){

    setbuf(stdout, NULL); // pra printar a execução na ordem certa e permitir novo processo

    unsigned long x = 0;
    for (unsigned long i = 0; i < _SC_ULONG_MAX; i++)
    {
       x+=i;
    }
    printf("job terminado \n");
    fflush(stdout);

    return 0;
}