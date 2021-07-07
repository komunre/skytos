#include "floppy.h"
#include "../src/port/port.h"

void init_floppy() {
    write_byte_to_port(DIGITAL_OUTPUT_REGISTER, 0x80);
    write_byte_to_port(DIGITAL_OUTPUT_REGISTER, 0x40);
    write_byte_to_port(DIGITAL_OUTPUT_REGISTER, 0x20);
    write_byte_to_port(DIGITAL_OUTPUT_REGISTER, 0x10);

    write_byte_to_port(DATA_FIFO, 7);
}

char* read_sector(unsigned short sector) {
    write_byte_to_port(DATA_FIFO, 6);
    char* data = "";
    data += read_byte_from_port(DIGITAL_INPUT_REGISTER);
    return data;
}