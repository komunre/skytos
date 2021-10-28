#include "ahci.h"
#include "../src/port/port.h"
#include "../src/tools.h"
#include "../src/screen.h"

char* HBA = 0x00;

int read_pci_word(char bus, char slot, char func, char offset) {
    int address;
    int lbus  = (int)bus;
    int lslot = (int)slot;
    int lfunc = (int)func;
    int tmp = 0;
 
    /* create configuration address as per Figure 1 */
    address = (int)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((int)0x80000000));

    write_byte_to_port(0xCF8, address);

    tmp = (short)((read_byte_from_port(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);

    return tmp;
}


void wrtie_pci(char bus, char slot, char func, char offset, short data) {
    int address;
    int lbus  = (int)bus;
    int lslot = (int)slot;
    int lfunc = (int)func;
    int tmp = 0;
 
    /* create configuration address as per Figure 1 */
    address = (int)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((int)0x80000000));

    write_byte_to_port(0xCF8, address);

    write_short_to_port(0xCFC, data);

    return tmp;
}

int read_pci_int(char bus, char slot, char func, char offset) {
    int address;
    int lbus  = (int)bus;
    int lslot = (int)slot;
    int lfunc = (int)func;
    int tmp = 0;
 
    address = (int)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((int)0x80000000));

    write_byte_to_port(0xCF8, address);

    tmp = ((read_byte_from_port(0xCFC) >> ((offset & 4) * 8)) & 0xFFFF);

    return tmp;
}

int bar5;
char* bar5addr;

char* ahci_control;

void init_ahci() {
    // Brute force device

    char ahci_bus = -1;
    char* ahci_device = -1;

    for (unsigned char i = 0; i < 255; i++) {
        for (char j = 0; j < 32; j++) {
            short vendor = read_pci_word(i, j, 0, 0);
            if (vendor != 0xFFFF) {
                short id = read_pci_word(i, j, 0, 2);
                char class = read_pci_word(i, j, 0x00, 0x08) & 0b00001111;
                print_str(intts(class));
                print_char(' ');
                if (class == 0x01) {
                    screenX = 0;
                    screenY = 1;
                    print_str("AHCI DETECTED");
                    ahci_bus = i;
                    ahci_device = j;
                    break;
                }
            }
        }
    }

    if (ahci_bus == -1 || ahci_device == -1) {
        screenY = 1;
        print_str("AHCI not initialized");
        return;
    }

    // Read BAR5

    clear_screen();
    
    print_str("BAR5: ");
    bar5 = read_pci_int(ahci_bus, ahci_device, 0, 0x24);
    print_str(intts(bar5));
    print_char(' ');
    bar5addr = (char*)(bar5 & 0xFFF0);
    print_str(intts(bar5addr[0] & 0b0100000));
    ahci_control = bar5addr[0];
    int version = chftoint(ahci_control[0x10], ahci_control[0x11], ahci_control[0x12], ahci_control[0x13]);
    print_str("Version: ");
    print_str(intts(version));
}