#ifndef SCREEN_H
#define SCREEN_H

extern char* video_mem;

extern int screenX;
extern int screenY;

extern int screenWidth;
extern int screenHeight;

void print_char_xy(char ch, int x, int y);

void print_char(char ch);

void print_str(char* str);

void clear_screen();

#endif
