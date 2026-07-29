#pragma once
#include <stdint.h>
namespace gdt {
    void init();
    void set_kernel_stack(uint32_t esp0);
}