#include "tools.h"

void sleep(int ticks) {
    for (int i = 0; i < ticks; i++) {
        asm ("nop");
    }
}

char* intts(int num) {
    int n = num;
    char* buffer;
    int i = 0;

    int isNeg = n<0;

    unsigned int n1 = isNeg ? -n : n;

    while(n1!=0)
    {
        buffer[i++] = n1%10+'0';
        n1=n1/10;
    }

    if(isNeg)
        buffer[i++] = '-';

    buffer[i] = '\0';

    for(int t = 0; t < i/2; t++)
    {
        buffer[t] ^= buffer[i-t-1];
        buffer[i-t-1] ^= buffer[t];
        buffer[t] ^= buffer[i-t-1];
    }

    if(n == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
    }   

    return buffer;
}

char* hexts(char hex) {
    
}