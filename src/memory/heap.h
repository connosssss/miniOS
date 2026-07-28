#pragma once
#include <stdint.h>
#include <stddef.h>

namespace heap {
    void init();
    void* kmalloc(size_t size);
    void kfree(void* ptr);
    bool self_test();
}
