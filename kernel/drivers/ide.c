#include "ahci.h"
#include "../src/screen.h"
#include "../src/port/port.h"

int* bar0;

void init_ide() {
    char ide_bus = -1;
    char* ide_device = -1;

    for (unsigned char i = 0; i < 255; i++) {
        for (char j = 0; j < 32; j++) {
            short vendor = read_pci_word(i, j, 0, 0);
            if (vendor != 0xFFFF) {
                short id = read_pci_word(i, j, 0, 2);
                short class_word = read_pci_word(i, j, 0x00, 0x09);
                char class = class_word & (0b111 << 8);
                char subclass = class_word & (0b111 << 16);
                print_str(intts(class));
                print_char(' ');
                print_str(intts(subclass));
                print_char(',');
                if (class == 0x01 && subclass == 0x01) {
                    print_str("IDE DETECTED");
                    ide_bus = i;
                    ide_device = j;
                    break;
                }
            }
        }
    }

    if (ide_bus == -1 || ide_device == -1) {
        print_str("IDE not initialized");
        return;
    }

    bar0 = read_pci_word(ide_bus, ide_device, 0, 0x10);

}

char* read_ide() {
    char data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = read_byte_from_port(bar0);
    }
    return data;
}