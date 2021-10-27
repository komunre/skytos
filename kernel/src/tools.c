#include "tools.h"
#include "screen.h"
#include "string.h"

char mem[4096];
long memIndex = 0;

char* allocate_index(long index) {
    return (char*)mem[index];
}

char* allocate(long size) {
    char* point = allocate_index(memIndex);
    memIndex += size;
    return point;
}

void sleep(int ticks) {
    for (int i = 0; i < ticks; i++) {
        asm ("nop");
    }
}

char* intts(int num) {
    if (num == 0) return "0";
    char* str = "";
    int index = 0;
    while(num > 0) {
        int digit = num % 10;
        num /= 10;
        str[index] = digit + 0x30;
        index++;
    }
    return reverse_size(str, index);
}

char* hexts(char hex) {
    
}

char* reverse_size(char* str, int size) {
    int index = 0;
    char* reversed = allocate(size);
    for (int i = size; i > 0; i--) {
        reversed[index] = str[i];
        index++;
    }
    return reversed;
}

char* reverse(char* str) {
    int len = 0;
    for (int i = 0; str[i] != 0x0; i++) {
        len++;
    }
    return reverse_size(str, len);
}