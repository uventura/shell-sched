// executavel compilado fora do make como int main, ta assim p n quebrar make
#include<stdio.h>
#include<unistd.h>
// para teste
void huge_for (){
    unsigned long x = 0;
    for (unsigned long i = 0; i < _SC_ULONG_MAX; i++)
    {
       x+=i;
    }
    printf("job terminado \n");
}