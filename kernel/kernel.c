#include "screen.h"

void kmain(void) {
    *video_mem = 'O';
    *(video_mem + 2) = 'K';

    print_str("Skytos loaded\nTest");
    while (1) {
        
    }
}