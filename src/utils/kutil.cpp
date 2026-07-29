#include "kutil.h"

namespace kutil {
    void hex_to_str(unsigned int val, char* out) {
        const char* digits = "0123456789abcdef";
        out[0] = '0';
        out[1] = 'x';
        
        for (int i = 0; i < 8; i++) {
            unsigned int shift = (7 - i) * 4;
            out[2 + i] = digits[(val >> shift) & 0xF];
        }
        
        out[10] = '\0';
    }

    void dec_to_str(unsigned int val, char* out) {
        char tmp[10];
        int n = 0;
        
        if (val == 0) {
            out[0] = '0';
            out[1] = '\0';
            return;
        }
        
        while (val > 0 && n < 10) {
            tmp[n++] = '0' + (val % 10);
            val /= 10;
        }
        
        for (int i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
        out[n] = '\0';
    }

    size_t strlen(const char* str) {
        size_t len = 0;
        while (str[len]) len++;
        return len;
    }

    int strcmp(const char* s1, const char* s2) {
        while (*s1 && (*s1 == *s2)) {
            s1++;
            s2++;
        }
        return static_cast<unsigned char>(*s1) - static_cast<unsigned char>(*s2);
    }

    void* memcpy(void* dest, const void* src, size_t n) {
        char* d = static_cast<char*>(dest);
        const char* s = static_cast<const char*>(src);
        for (size_t i = 0; i < n; i++) d[i] = s[i];
        return dest;
    }

    void* memset(void* s, int c, size_t n) {
        unsigned char* p = static_cast<unsigned char*>(s);
        for (size_t i = 0; i < n; i++) p[i] = static_cast<unsigned char>(c);
        return s;
    }
}
