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
 
    /* create configuration address as per Figure 1 */
    address = (int)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((int)0x80000000));

    write_byte_to_port(0xCF8, address);

    write_short_to_port(0xCFC, data);
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

char implemented_ports[32];

int* ahci_control;

int* get_port_addr(int offset) {
    return (char*)(0x100 + (offset * 0x80));
}

void init_ahci() {
    // Brute force device

    char ahci_bus = -1;
    char* ahci_device = -1;

    for (unsigned char i = 0; i < 255; i++) {
        for (char j = 0; j < 32; j++) {
            short vendor = read_pci_word(i, j, 0, 0);
            if (vendor != 0xFFFF) {
                short id = read_pci_word(i, j, 0, 2);
                short class_word = read_pci_word(i, j, 0x00, 0x09);
                char class = class_word & 0b000000000000111;
                print_str(intts(class));
                print_char(' ');
                if (class == 0x01) {
                    print_str("AHCI DETECTED");
                    ahci_bus = i;
                    ahci_device = j;
                    break;
                }
            }
        }
    }

    if (ahci_bus == -1 || ahci_device == -1) {
        print_str("AHCI not initialized");
        return;
    }

    // Read BAR5

    clear_screen();
    
    print_str("BAR5: ");
    bar5 = read_pci_word(ahci_bus, ahci_device, 0, 0x24);
    print_str(intts(bar5));
    print_char(' ');
    bar5addr = (char*)(bar5 & 0xFFFFFFF0);
    print_str(intts(bar5addr[0] & 0b0100000));
    ahci_control = bar5addr;
    print_str(intts(ahci_control[0x10]));
    int version = chftoint(ahci_control[0x10], ahci_control[0x11], ahci_control[0x12], ahci_control[0x13]);
    print_str("Version: ");
    print_str(intts(version));
    
    wrtie_pci(ahci_bus, ahci_device, 0, 0x04, (short)0b0000000000000111);
    print_str("PCI 0x04: ");
    print_bits(read_pci_word(ahci_bus, ahci_device, 0, 0x04));

    ahci_control[0x00] |= 0b0 << 18;

    print_str("SAM: ");
    print_str(ahci_control[0x00] & (0b1 << 18) ? "Yes" : "No");

    ahci_control[0x04] = 0b1 << 1;

    print_str("Restart: ");
    print_str(ahci_control[0x04] & 1 ? "Yes" : "No");

    ahci_control[0x04] &= ~(1 << 31);
    ahci_control[0x04] &= ~(1 << 1);
    ahci_control[0x04] &= ~(0b1);

    print_str("Restart: ");
    print_str(ahci_control[0x04] & 1 ? "Yes" : "No");

    //ahci_control[0x04] = 0;
    ahci_control[0x04] |= 1 << 31; // Enable AHCI (technically 0b000000000000000000000000000000000001)
    print_str("AE enabled: ");
    print_str((ahci_control[0x04] & (0b1 << 31)) ? "Yes" : "No");

    //ahci_control[0x04] &= ~(0b1 << 31);
    ahci_control[0x04] |= 0b1 << 1;

    print_str(", ");

    print_str("AE check: ");
    print_str((ahci_control[0x04] & (0b1 << 31)) ? "Yes" : "No");

    print_str(", ");

    print_str("IE enabled: ");
    print_str((ahci_control[0x04] & (0b1 << 1)) ? "Yes" : "No");

    sleep(100000);
    screenY = 5;
    print_str("Total bits: ");
    print_bits(ahci_control[0x04]);

    int CAP = ahci_control[0x04];

    print_str("Total ports: ");
    print_str(intts(chftoint(CAP, CAP & 1, CAP & 2, CAP & 3)));

    print_str("AE check: ");
    print_str((ahci_control[0x04] & (0b1 << 31))  ? "Yes" : "No");

    int ports = ahci_control[0xC];

    char impl_index = 0;

    screenY = 10;
    for (int i = 0; i < 32; i++) {
        if (ports & (0b1 << i)) {
            print_char('1');
            implemented_ports[impl_index] = i;
            impl_index++;
        }
        else {
            print_char('0');
        }
    }
}

int clb[1024];
int fb[256];
int 

short* read_ahci() {
    /*char* port = get_port_addr(0);
    port[0x18] &= ~(0b1);
    port[0x18] &= ~(0b1 << 8);
    while ((port[0x18] & (0b1 << 14)) || (port[0x18] & (0b1 << 15))) {} // wait for clear
    port[0x00] = &clb; // Allocate command list buffer
    port[0x08] = &fb; // Allocate FIS buffer*/

    

}