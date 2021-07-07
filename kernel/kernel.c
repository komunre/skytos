#include "src/screen.h"
#include "src/string.h"
#include "src/ps2.h"

char chars[255] = "-=0\tqwertyuiop[]00asdfghjkl;'`0\\zxcvbnm,./0*";

void kmain(void) {
    print_str(format_str("Skytos loaded\n"));
    
    /*for (int i = 0; i < 255; i++) {
        print_char((char)i);
    }*/
    print_char(get_resp(0x60));
    while (1) {
        char ch = get_resp(0x60);
        if (ch < 0x81) {
            if (ch == 0x01) {
                // Escape
            }

            if (ch > 0x02 && ch < 0x0B) {
                print_char((ch - 0x01) + '0');
            }

            char actual_char;
            if (ch > 0x0B && ch < 0x37) {
                if (ch == 0x0E) {
                    screenX--;
                }
                else {
                    actual_char = chars[ch - 0x0C];
                    print_char(actual_char);
                }
            }

            for (int i = 0; i < 10000000; i++) {
                asm ("nop");
            }
        }
    }
}