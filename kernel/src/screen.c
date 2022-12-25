#include "screen.h"

char* video_mem = (char*)0xb8000;

int screenX = 0;
int screenY = 0;

int screenWidth = 80;
int screenHeight = 25;

void print_char_xy(char ch, int x, int y) {
    video_mem [(x * 2) + (y * screenWidth * 2)] = ch;
}

void print_char(char ch) {
    if (ch == '\n') {
        screenY++;
        return;
    }
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
    for (int i = 0; i < 100; i++) {
        if (str[i] == 0x00 || str[i] == 0) {
            break;
        }
        print_char(str[i]);
    }
}

void clear_screen() {
    screenX = 0;
    screenY = 0;
    for (int i = 0; i < screenWidth * screenHeight; i++) {
        print_char(' ');
    }
    screenX = 0;
    screenY = 0;
}