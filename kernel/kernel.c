#include "src/screen.h"
#include "src/string.h"
#include "src/ps2.h"
#include "src/keyboard.h"
#include "drivers/floppy.h"
#include "src/tools.h"
#include "drivers/ide.h"

void kmain(void) {
    //init_drive();
    //print_str("Drive init done(?) ");
    //char* data = read_sector(0);
    //print_str(data);

    //print_str("Letting AHCI initialize... ");
    //sleep(100000);

    //print_str("\nAHCI: ");
    //init_ahci();

    //print_str("\nIDE: ");
    //init_ide();

    print_str(format_str(" Skytos loaded\n"));
    print_str("A number! ");
    print_str(intts(15567));
    
    /*for (int i = 0; i < 255; i++) {
        print_char((char)i);
    }*/
    while (1) {
        process_key();
    }
}