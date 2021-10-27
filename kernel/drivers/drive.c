#include "drive.h"
#include "../src/port/port.h"
#include "../src/tools.h"

char* baseIO = 0x1F0;

void init_drive() {
    write_short_to_port(baseIO[6], 0xF0);
    write_byte_to_port(baseIO[1], 0x00);
}

int wait_status() {
    char status = read_byte_from_port(baseIO[7]);
    while ((status) & 0b1 != 1 && (status) & 0b0000001 != 0x0) {}
    if ((status & 0b0000001) != 0x0) {
        print_str("Error!!");
    }
}

short* read_drive_data() {
    write_byte_to_port(baseIO[2], 1);
    write_byte_to_port(baseIO[7], 0x20);
    wait_status();
    short data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = read_short_from_port(baseIO[0]);
    }
    return data;
}

void write_drive_data(short data) {
    write_byte_to_port(baseIO[2], 1);
    write_byte_to_port(baseIO[7], 0x30);
    wait_status();
    for (int i = 0; i < 256; i++) {
        write_short_to_port(baseIO[0], data);
        sleep(2);
    }
    write_byte_to_port(baseIO[7], 0xE7);
}