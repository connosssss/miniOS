#pragma once

#include <stdint.h>

namespace pmm {
    void init(uint32_t multiboot_info_addr);
    uint32_t alloc_frame();
    void free_frame(uint32_t physical_addr);
    uint32_t free_frames();
    uint32_t total_frames();
}
