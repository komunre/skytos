#include "drive.h"
#include "../src/port/port.h"
#include "../src/tools.h"
#include "../src/screen.h"
#include "../src/string.h"

char* baseIO = 0x1F0;

char* control = 0x3F7;

char drive = 0b00001101;

void init_drive() {
    print_str("Iniializing drive!");
    write_short_to_port(baseIO[6], drive);
    //write_byte_to_port(baseIO[4], 0);
    //write_byte_to_port(baseIO[5], 0);
    write_byte_to_port(baseIO[7], 0xEC);
    //write_byte_to_port(baseIO[7], 0x08);
    print_str("Reading status...");
    wait_status();
    if (read_byte_from_port(baseIO[7]) == 0) {
        print_str("NO DRIVE!");
    }

    check_active_drive();
}

void check_active_drive() {
    char drive = read_byte_from_port(control[1]);
    if (drive & 0x01 == 0) {
        print_str("drive number 1 ");
    }
    if (drive & 0x00 == 0) {
        print_str("drive number 0 ");
    }
}

int wait_status() {
    char status = read_byte_from_port(baseIO[7]);
    long cycles = 0;
    const long max_cycles = 100000;
    while (status & 0b00000001 == 1 && status & 0b00000010 == 1 && cycles < max_cycles) {
        status = read_byte_from_port(baseIO[7]);
        screenX = 0;
        screenY = 1;
        print_str(intts(status));
        print_str(intts(cycles));
        cycles++;
    }
    if (cycles >= max_cycles) {
        screenX = 0;
        screenY = 0;
        print_str("restart");
        write_byte_to_port(control, 0b00100000);
    }
    if ((status & 0b00000010) == 1) {
        print_str("Success!");
    }
    if (status & 0b10000000 == 1 || (status & 0b00000010) == 0) {
        print_str("Error?");
        char error = read_byte_from_port(baseIO[1]);
        print_char(intts(error));
        if (error & 0b10000000 == 1) {
            print_str("No address mark");
        }
        if (error & 0b01000000 == 1) {
            print_str("Track zero not found");
        }
        if (error & 0b00100000 == 1) {
            print_str("Aborted");
        }
        if (error & 0b00010000 == 1) {
            print_str("Media change");
        }
        if (error & 0b00001000 == 1) {
            print_str("ID not found");
        }
        if (error & 0b00000100 == 1) {
            print_str("Media changed");
        }
        if (error & 0b00000010 == 1) {
            print_str("Data error");
        }
        if (error & 0b00000001 == 1) {
            print_str("Bad block");
        }
    }
    if (status & 0b00000100 == 1) {
        print_str("Drive fault!");
    }
}

void seek(char sectors) {
    write_byte_to_port(baseIO[0], sectors);
    write_byte_to_port(baseIO[7], 0x70);
}

short* read_drive_data(char sector) {
    write_byte_to_port(baseIO[6], drive);
    write_byte_to_port(baseIO[4], sector);
    write_byte_to_port(baseIO[5], sector);
    write_byte_to_port(baseIO[2], 1);
    write_byte_to_port(baseIO[7], 0x20);
    wait_status();
    short data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = read_short_from_port(baseIO[0]);
    }
    check_active_drive();
    return data;
}

void check_writing() {
    if (read_byte_from_port(control[6]) == 0) {
        print_str("Writing");
    }
}

void write_drive_data(short data, char sector) {
    write_byte_to_port(baseIO[4], sector);
    write_byte_to_port(baseIO[5], sector);
    write_byte_to_port(baseIO[2], 1);
    write_byte_to_port(baseIO[7], 0x30);
    wait_status();
    for (int i = 0; i < 256; i++) {
        write_short_to_port(baseIO[0], data);
        sleep(2);
    }
    write_byte_to_port(baseIO[7], 0xE7);
    check_writing();
    
}