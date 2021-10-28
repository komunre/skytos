#ifndef TOOLS_H
#define TOOLS_H

void sleep(int ticks);

char* intts(int num);


char* hexts(char hex);

char* reverse(char* str);

char* allocate_index(long index);
char* allocate();

char* reverse_size(char* str, int size);

int chftoint(char b1, char b2, char b3, char b4);

void print_bits(int num);
void print_bits_short(short num);



#endif