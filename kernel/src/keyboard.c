#include "keyboard.h"
#include "ps2.h"
#include "screen.h"

char chars[255] = "-=0\tqwertyuiop[]00asdfghjkl;'`0\\zxcvbnm,./0*";
char* command = "";
int command_counter = 0;

void process_key() {
    char ch = get_resp(0x60);
    if (ch < 0x81) {
        if (ch == 0x01) {
            // Escape
        }

        if (ch > 0x02 && ch < 0x0B) {
            command += (ch - 0x01) + '0';
            print_char((ch - 0x01) + '0');
        }

        char actual_char;
        if (ch > 0x0B && ch < 0x37) {
            if (ch == 0x0E) {
                screenX--;
                print_char_xy(' ', screenX, screenY);
            }
            else if (ch == 0x1C) {
                // Send command
            }
            else {
                actual_char = chars[ch - 0x0C];
                command += actual_char;
                print_char(actual_char);
            }
        }

        if (ch == 0x3A) {
            print_char(' ');
        }

        for (int i = 0; i < 7000000; i++) {
            asm ("nop");
        }
    }
}