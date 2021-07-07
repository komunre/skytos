#ifndef FLOPPY_H
#define FLOPPY_H

enum FloppyRegisters {
    STATUS_REGISTER_A = 0x3F0,
    STATUS_REGISTER_B = 0x3F1,
    DIGITAL_OUTPUT_REGISTER = 0x3F2,
    TAPE_DRIVE_REGISTER = 0x3F3,
    MAIN_STATUS_REGUSTER = 0x3F4,
    DATARATE_SELECT_REGISTER  = 0x3F4,
    DATA_FIFO = 0x3F5,
    DIGITAL_INPUT_REGISTER = 0x3F7,
    CONFIGURATION_CONTROL_REGISTER = 0x3F7
};

void init_floppy();

char* read_sector(unsigned short sector);



#endif