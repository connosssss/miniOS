#pragma once
#include <stddef.h>

namespace kutil {
    void hex_to_str(unsigned int val, char* out);
    void dec_to_str(unsigned int val, char* out);
    size_t strlen(const char* str);
    int strcmp(const char* s1, const char* s2);
    void* memcpy(void* dest, const void* src, size_t n);
    void* memset(void* s, int c, size_t n);
}
