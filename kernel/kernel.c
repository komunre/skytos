#include "src/screen.h"
#include "src/string.h"
#include "src/ps2.h"
#include "src/keyboard.h"
#include "drivers/floppy.h"
#include "src/tools.h"

void kmain(void) {
    print_str("initializing floppies...");
    sleep(30000000);
    init_floppy();
    char* data = read_sector(0);
    print_str(data);

    print_str(format_str("Skytos loaded\n"));
    
    /*for (int i = 0; i < 255; i++) {
        print_char((char)i);
    }*/
    print_char(get_resp(0x60));
    while (1) {
        process_key();
    }
}