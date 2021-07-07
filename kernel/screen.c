#include "screen.h"

char* video_mem = (char*)0xb8000;

int screenX = 0;
int screenY = 0;

int screenWidth = 80;
int screenHeight = 25;

void print_char_xy(char ch, int x, int y) {
    video_mem [(x * 2) + (y * screenWidth)] = ch;
}

void print_char(char ch) {
    print_char_xy(ch, screenX, screenY);
    screenX++;
    if (screenX > screenWidth) {
        screenX = 0;
        screenY++;
        if (screenY > screenHeight) {
            screenY = 0;
        }
    }
}

void print_str(char* str) {
    for (int i = 0; i < sizeof(str) / sizeof(str[0]); i++) {
        print_char(str[i]);
    }
}
