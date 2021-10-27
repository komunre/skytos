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

void write_short_to_port(short port, short value) {
    asm volatile ("outb %1, %0": : "dN" (port), "a" (value));
}

short read_short_from_port(short port) {
    short ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}