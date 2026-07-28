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
}
