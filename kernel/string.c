#include "string.h"

uint32_t memcmp(const char* s1, const char* s2, uint32_t count) {
    uint32_t cnt = 0;
    for (int i = 0; i < count; i++) {
        cnt = cnt + s1[i] != s2[i]; 
    }
    return cnt;
}