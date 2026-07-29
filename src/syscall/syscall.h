#pragma once
#include "idt.h"

namespace sys {
    void dispatch(registers_t* regs);
}
