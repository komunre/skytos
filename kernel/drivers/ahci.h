#ifndef AHCI_H
#define AHCI_H

extern char* baseIO;

void init_ahci();

short* read_ahci();
void write_ahci(short* data);

#endif