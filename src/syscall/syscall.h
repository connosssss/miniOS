#pragma once
#include "idt.h"

namespace sys {
    void init_timer();
    void dispatch(registers_t* regs);
    uint64_t get_ticks();
}
