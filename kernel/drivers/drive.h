#ifndef DRIVE_H
#define DRIVE_H

extern char* baseIO;

void init();

short* read_data();
void write_data(short* data);

#endif