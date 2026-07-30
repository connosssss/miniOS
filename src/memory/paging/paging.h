#pragma once
#include <stdint.h>
#include <stddef.h>

namespace paging {
    constexpr uint32_t PAGE_PRESENT = 0x1;
    constexpr uint32_t PAGE_RW      = 0x2;
    constexpr uint32_t PAGE_USER    = 0x4;
    constexpr uint32_t PAGE_SIZE    = 4096;

    void init();
    uint32_t create_user_page_directory();
    void switch_page_directory(uint32_t pd_phys);
    uint32_t get_kernel_page_directory();
    void set_user_accessible(uint32_t virt_addr, uint32_t size);
    void set_user_accessible_in_dir(uint32_t pd_phys, uint32_t virt_addr, uint32_t size);
}

