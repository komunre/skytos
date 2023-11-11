#include "port.h"

void write_byte_to_port(short port, char value) 
{
    asm volatile ("outb %1, %0": : "dN" (port), "a" (value));
}

char read_byte_from_port(short port) {
    char ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}