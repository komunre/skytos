#ifndef STRING_H
#define STRING_H

#include "stdint.h"

uint32_t memcmp(const char* s1, const char* s2, uint32_t count);
uint32_t memcpy(int8_t* dest, int8_t* src, uint32_t count);

#endif