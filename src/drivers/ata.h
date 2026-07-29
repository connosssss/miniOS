#pragma once
#include <stdint.h>
#include <stdbool.h>

namespace ata {
    bool init();

    // Read 'count' 512-byte sectors starting at LBA 'lba' into 'buffer'.
    bool read_sectors(uint32_t lba, uint32_t count, void* buffer);
    // Write 'count' 512-byte sectors from 'buffer' to disk starting at LBA 'lba'.
    bool write_sectors(uint32_t lba, uint32_t count, const void* buffer);
    //both return true on success


    uint32_t sector_count();

    constexpr uint32_t SECTOR_SIZE = 512;
}


