#include "stdio.h"
#include "x86.h"

void putc(char c) {
    x86_Video_WriteCharTeletype(c, 0);
}
void puts(const char* s) {
    while (*s) {
        putc(*s);
        s++;
    }
}

#define PRINTF_STATE_NORMAL 0
#define PRINTF_STATE_PARAMETER 1
#define PRINTF_STATE_SPECIFIER 2

const char g_HexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
int* printf_number(int* argp, int length, bool sign, int radix) {
    char buffer[32];
    unsigned long number;
    int number_sign = 1;
    int pos = 0;
    
    int n = *argp;
    if (sign) {
        if (n < 0) {
            n = -n;
            number_sign = -1;
        }
        number = n;
    }
    else {
        number = *(unsigned int*)argp;
    }
    argp++;
    
    do {
        uint32_t rem;
        x86_div64_32(number, radix, &(uint64_t)number, &(uint32_t)rem);
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);
    if (sign && number_sign < 0) {
        buffer[pos++] = '-';
    }
    while (--pos >= 0) {
        putc(buffer[pos]);
    }
    return argp;
}

void _cdecl printf(const char* fmt, ...) {
    int* argp = (int*)&fmt;
    int state = PRINTF_STATE_NORMAL;
    int length = 0;
    int radix = 10;
    bool sign = false;

    argp++;

    while (*fmt) {
        switch (state) {
            case PRINTF_STATE_NORMAL:
                switch (*fmt) {
                    case '%':   state = PRINTF_STATE_PARAMETER;
                                break;
                    default:    putc(*fmt);
                                break;
                }
                break;
            case PRINTF_STATE_PARAMETER:
                switch (*fmt) {
                    default:    state = PRINTF_STATE_SPECIFIER;
                                goto PRINTF_STATE_SPEC_;
                }
                break;
            case PRINTF_STATE_SPECIFIER:
            PRINTF_STATE_SPEC_:
                switch (*fmt) {
                    case 'c':   putc((char)*argp);
                                argp++;
                                break;
                    case 's':   puts(*(char**)argp);
                                argp++;
                                break;
                    case '%':   putc('%');
                                break;
                    case 'd':
                    case 'i':   radix = 10; sign = true;
                                argp=printf_number(argp, 0, sign, radix);
                                break;
                    case 'u':   radix = 10; sign = false;
                                argp=printf_number(argp, 0, sign, radix);
                                break;
                    case 'X':
                    case 'x':
                    case 'p':   radix = 16; sign = false;
                                argp=printf_number(argp, 0, sign, radix);
                                break;
                    case 'o':   radix = 8; sign = false;
                                argp=printf_number(argp, 0, sign, radix);
                                break;
                    // ignore invalid
                    default:    break;
                }
                state = PRINTF_STATE_NORMAL;
                length = 0;
                radix = 10;
                sign = false;
                break;
        }
        fmt++;
    }
}