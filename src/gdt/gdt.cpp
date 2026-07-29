#include "gdt.h"

namespace {
    struct gdt_entry {
        uint16_t limit_low;
        uint16_t base_low; //lower 16 bits of base address
        uint8_t base_middle; // middle 8
        uint8_t access;
        uint8_t granularity; //flags + high 4 bits of the limit
        uint8_t base_high; //upper 8
    } __attribute__((packed));


    struct gdt_ptr {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    struct tss_entry_t {
        uint32_t prev_tss;
        uint32_t esp0; // Ring 0 stack pointer 
        uint32_t ss0;  // Ring 0 stack selector (0x10)
        uint32_t unused[23];
    } __attribute__((packed));


    // null descriptor +kernel code + data + user code + data + tss
    gdt_entry entries[6];
    gdt_ptr ptr;
    tss_entry_t tss;

    void set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity){
        entries[num].base_low = base & 0xFFFF;
        entries[num].base_middle = (base >> 16) & 0xFF;
        entries[num].base_high = (base >> 24) & 0xFF;
        entries[num].limit_low = limit & 0xFFFF;
        entries[num].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
        entries[num].access = access;
    }
}


extern "C" void gdt_flush(uint32_t gdt_ptr_address);
extern "C" void tss_flush(uint32_t tss_selector);

namespace gdt {
    void set_kernel_stack(uint32_t esp0) {
        tss.esp0 = esp0;
    }

    void init() {
        ptr.limit = sizeof(entries) - 1;
        ptr.base  = reinterpret_cast<uint32_t>(&entries);

        set_gate(0, 0, 0, 0, 0);
        set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);        
        set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);        
        set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
        set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

        // Setup TSS
        uint32_t tss_base = reinterpret_cast<uint32_t>(&tss);
        uint32_t tss_size = sizeof(tss);
        set_gate(5, tss_base, tss_size - 1, 0x89, 0x00); // 0x28 TSS Descriptor

        tss.ss0 = 0x10;

        gdt_flush(reinterpret_cast<uint32_t>(&ptr));
        tss_flush(0x28); // Load TSS selector into TR
    }
}


