// Physical Memory Manager (PMM) with dynamic bitmap sizing.
// Detects total physical memory dynamically from the Multiboot memory map,
// places the bitmap array in RAM immediately following the kernel image (_ker2nel_end),
// and reserves memory covering low memory (<1MB), the kernel, and the bitmap itself.
#include "pmm.h"
#include "multiboot.h"
#include "terminal.h"

extern "C" uint8_t _kernel_end;

namespace {
    constexpr uint32_t FRAME_SIZE = 4096;

    uint8_t* bitmap = nullptr;
    uint32_t max_frames = 0;

    inline void set_used(uint32_t frame) { 
        if (frame < max_frames && bitmap) {
            bitmap[frame / 8] |= (1 << (frame % 8)); 
        }
    }

    inline void set_free(uint32_t frame) { 
        if (frame < max_frames && bitmap) {
            bitmap[frame / 8] &= ~(1 << (frame % 8)); 
        }
    }

    inline bool is_used(uint32_t frame) {
        if (frame >= max_frames || !bitmap) return true;
        return bitmap[frame / 8] & (1 << (frame % 8));
    }

    uint32_t align_up(uint32_t addr, uint32_t align) {
        return (addr + align - 1) & ~(align - 1);
    }
}

namespace pmm {
    void init(uint32_t multiboot_info_addr) {
        auto* mbi = reinterpret_cast<multiboot_info_t*>(multiboot_info_addr);

        uint64_t highest_addr = 0;


        if (mbi->flags & (1 << 6)) { // bit 6 = mmap_addr/mmap_length are valid
            uint32_t addr = mbi->mmap_addr;
            uint32_t end  = mbi->mmap_addr + mbi->mmap_length;

            while (addr < end) {
                auto* entry = reinterpret_cast<multiboot_mmap_entry_t*>(addr);

                if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                    uint64_t region_end = entry->addr + entry->len;

                    if (region_end > highest_addr) {
                        highest_addr = region_end;
                    }
                }

                addr += entry->size + sizeof(entry->size);
            }
        } 
        
        else {
            terminal::write("[pmm] WARNING: bootloader provided no memory map.\n");
        }

        // Cap at 4GB physical address space for 32 bit architecture
        if (highest_addr > 0x100000000ULL) {
            highest_addr = 0x100000000ULL;
        }

        max_frames = static_cast<uint32_t>(highest_addr / FRAME_SIZE);
        uint32_t bitmap_size = (max_frames + 7) / 8;

        uint32_t kernel_end_addr = reinterpret_cast<uint32_t>(&_kernel_end);
        uint32_t bitmap_start_addr = align_up(kernel_end_addr, FRAME_SIZE);
        bitmap = reinterpret_cast<uint8_t*>(bitmap_start_addr);

        // mark all frames as used
        for (uint32_t i = 0; i < bitmap_size; i++) {
            bitmap[i] = 0xFF;
        }

        // Then mark all available frames as free
        if (mbi->flags & (1 << 6)) {
            uint32_t addr = mbi->mmap_addr;
            uint32_t end  = mbi->mmap_addr + mbi->mmap_length;

            while (addr < end) {
                auto* entry = reinterpret_cast<multiboot_mmap_entry_t*>(addr);

                if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                    uint64_t region_end = entry->addr + entry->len;
                    for (uint64_t a = entry->addr; a + FRAME_SIZE <= region_end; a += FRAME_SIZE) {
                        uint32_t frame = static_cast<uint32_t>(a / FRAME_SIZE);
                        if (frame < max_frames) set_free(frame);
                    }
                }

                addr += entry->size + sizeof(entry->size);
            }
        }



        
        uint32_t reserved_end = align_up(bitmap_start_addr + bitmap_size, FRAME_SIZE);

        for (uint32_t a = 0; a < reserved_end; a += FRAME_SIZE) {
            uint32_t frame = a / FRAME_SIZE;
            if (frame < max_frames) set_used(frame);
        }

        terminal::write("pmm initialized: ");
        terminal::write_dec(free_frames() * (FRAME_SIZE / 1024));
        terminal::write(" KB free of ");
        terminal::write_dec(total_frames() * (FRAME_SIZE / 1024));
        terminal::write(" KB tracked.\n");
    }




    uint32_t alloc_frame() {
        for (uint32_t i = 0; i < max_frames; i++) {
            if (!is_used(i)) {
                set_used(i);
                return i * FRAME_SIZE;
            }
        }
        return 0; // out of memory
    }


    void free_frame(uint32_t physical_addr) {
        set_free(physical_addr / FRAME_SIZE);
    }


    uint32_t free_frames() {
        uint32_t count = 0;
        for (uint32_t i = 0; i < max_frames; i++) {
            if (!is_used(i)) count++;
        }
        return count;
    }


    uint32_t total_frames() { return max_frames; }
}

