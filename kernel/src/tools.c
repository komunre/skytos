#include "tools.h"

void sleep(int ticks) {
    for (int i = 0; i < ticks; i++) {
        asm ("nop");
    }
}