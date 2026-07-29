#pragma once
#include <stdint.h>

namespace vfs {
    void init(uint32_t multiboot_info_addr);
    int32_t read_file(const char* name, char* buf, uint32_t max_len);
    int32_t list_files(char* buf, uint32_t max_len);
}
