#pragma once
#include <stdint.h>

struct registers_t {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

typedef void (*irq_handler_t)(registers_t*);

namespace idt {
    void init();
    // Register a handler for hardware IRQs ie 1-> keyboard (0-15)
    void install_irq_handler(int irq, irq_handler_t handler);
}
