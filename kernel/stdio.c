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
#define PRINTF_STATE_LENGTH_SHORT 3
#define PRINTF_STATE_LENGTH_LONG 4

#define PRINTF_LENGTH_SHORT_SHORT 2
#define PRINTF_LENGTH_SHORT 1
#define PRINTF_LENGTH_DEFAULT 0
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4

const char g_HexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
int* printf_number(int* argp, int length, bool sign, int radix) {
    char buffer[32];
    uint64_t number;
    int number_sign = 1;
    int pos = 0;
    
    switch (length) {
        case PRINTF_LENGTH_SHORT_SHORT:
        case PRINTF_LENGTH_SHORT:
        case PRINTF_LENGTH_DEFAULT:
            if (sign) {
                int n = *argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned int)n;
            }
            else {
                number = *(unsigned int*)argp;
            }
            argp++;
            break;
        case PRINTF_LENGTH_LONG:
            if (sign) {
                long int n = *(long int*)argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long int)n;
            }
            else {
                number = *(unsigned long int*)argp;
            }
            argp += 2;
            break;
        case PRINTF_LENGTH_LONG_LONG:
            if (sign) {
                long long n = *(long long*)argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            }
            else {
                number = *(unsigned long long*)argp;
            }
            argp += 4;
            break;
    }
    //putc(g_HexChars[length]);
    
    do {
        uint32_t rem;
        x86_div64_32(number, radix, &(uint64_t)number, &(uint32_t)rem);
        if (rem > 15 || rem < 0) {
            buffer[pos++] = '?';
            continue; 
        }
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
                    case 'h': 
                        length = PRINTF_LENGTH_SHORT;
                        state = PRINTF_STATE_LENGTH_SHORT;
                        break;
                    case 'l':
                        length = PRINTF_LENGTH_LONG;
                        state = PRINTF_STATE_LENGTH_LONG;
                        break;
                    default:    state = PRINTF_STATE_SPECIFIER;
                                goto PRINTF_STATE_SPEC_;
                }
                break;
            case PRINTF_STATE_LENGTH_SHORT:
                if (*fmt == 'h') {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPECIFIER;
                }
                else {
                state = PRINTF_STATE_SPECIFIER;
                goto PRINTF_STATE_SPEC_;
                }
                break;
            case PRINTF_STATE_LENGTH_LONG:
                if (*fmt == 'l') {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPECIFIER;
                }
                else {
                state = PRINTF_STATE_SPECIFIER;
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
                                argp=printf_number(argp, length, sign, radix);
                                break;
                    case 'u':   radix = 10; sign = false;
                                argp=printf_number(argp, length, sign, radix);
                                break;
                    case 'X':
                    case 'x':
                    case 'p':   radix = 16; sign = false;
                                argp=printf_number(argp, length, sign, radix);
                                break;
                    case 'o':   radix = 8; sign = false;
                                argp=printf_number(argp, length, sign, radix);
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