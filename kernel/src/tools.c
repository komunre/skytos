#include "tools.h"
#include "screen.h"
#include "string.h"

char mem[4096];
long memIndex = 0;

char* allocate_index(long index) {
    return &(mem[index]);
}

char* allocate(long size) {
    char* point = allocate_index(memIndex);
    memIndex += size;
    return point;
}

void free(long index, long size) {
    for (int i = index; i < size + index; i++) {
        mem[i] = 0;
    }
    memIndex = index; // ???
}

void sleep(int ticks) {
    for (int i = 0; i < ticks; i++) {
        asm ("nop");
    }
}

char* intts(int num) {
    if (num == 0) return "0";
    char str[10];
    int index = 0;
    if (num < 0) {
        str[0] = '-';
        index++;
        num *= -1;
    }
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

int chftoint(char b1, char b2, char b3, char b4) {
    return b1 << 24 | b2 << 16 | b3 << 8 | b4;
}

int lsr(int x, int n)
{
  return (int)((unsigned int)x >> n);
}

int logical_right_shift(int x, int n)
{
    int size = sizeof(int) * 8; // usually sizeof(int) is 4 bytes (32 bits)
    return (x >> n) & ~(((0x1 << size) >> n) << 1);
}

void print_bits(int num) {
    for (int i = 0; i < 32; i++) {
        print_char((num & (0b1 << i)) ? '1' : '0');
    }
}