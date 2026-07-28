#include "paging.h"
#include "pmm.h"
#include "terminal.h"
#include "serial.h"

extern "C" void load_page_directory(uint32_t*);
extern "C" void enable_paging();

namespace {
    constexpr uint32_t ENTRIES_PER_TABLE = 1024;
    constexpr uint32_t PAGE_SIZE = 4096;
    constexpr uint32_t TABLE_SPAN = ENTRIES_PER_TABLE * PAGE_SIZE; // 4MB per page table

    alignas(4096) uint32_t page_directory[ENTRIES_PER_TABLE];
    uint32_t num_tables_mapped = 0;
}

namespace paging {
    void init() {
        for (uint32_t i = 0; i < ENTRIES_PER_TABLE; i++) {
            page_directory[i] = 0;
        }

        uint32_t total_bytes = pmm::total_frames() * PAGE_SIZE;
        if (total_bytes == 0) {
            total_bytes = 256 * 1024 * 1024; // Fallback to 256MB if PMM hasn't run
        }

      
        uint32_t tables_needed = (total_bytes + TABLE_SPAN - 1) / TABLE_SPAN;
        if (tables_needed > 1024) tables_needed = 1024; // Cap at 4GB because 32 bit
        num_tables_mapped = tables_needed;

        for (uint32_t t = 0; t < tables_needed; t++) {

            //  allocate a physical frame for this page table
            uint32_t table_frame = pmm::alloc_frame();
            auto* table = reinterpret_cast<uint32_t*>(table_frame);

            for (uint32_t i = 0; i < ENTRIES_PER_TABLE; i++) {
                uint32_t phys = t * TABLE_SPAN + i * PAGE_SIZE;
                table[i] = phys | PAGE_PRESENT | PAGE_RW; //supervisor default
            }

            
            page_directory[t] = table_frame | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }

     
        load_page_directory(page_directory);
        enable_paging();

        terminal::write("paging dynamically identity-mapped ");
        terminal::write_dec(tables_needed * 4);
        terminal::write("MB physical RAM from PMM.\n");

        serial::write("paging dynamically identity-mapped ");
        serial::write_dec(tables_needed * 4);
        serial::write("MB physical RAM from PMM.\n");
    }





    void set_user_accessible(uint32_t virt_addr, uint32_t size) {

        uint32_t start_page = virt_addr / PAGE_SIZE;
        uint32_t end_page = (virt_addr + size - 1) / PAGE_SIZE;

        for (uint32_t page = start_page; page <= end_page; page++) {
            uint32_t t = page / ENTRIES_PER_TABLE;
            uint32_t i = page % ENTRIES_PER_TABLE;

            if (t < num_tables_mapped && (page_directory[t] & PAGE_PRESENT)) {
                uint32_t table_phys = page_directory[t] & ~0xFFF;
                auto* table = reinterpret_cast<uint32_t*>(table_phys);
                table[i] |= PAGE_USER;
            }

        }
    }
}
