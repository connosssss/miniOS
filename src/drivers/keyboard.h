#pragma once
#include <stdint.h>
namespace keyboard {
    void init();
    bool try_read(char* out_char);
}