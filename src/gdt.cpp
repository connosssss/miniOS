

#include "gdt.h"
#include <stdint.h>


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


    gdt_entry entries[3];
    gdt_ptr ptr;


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


namespace gdt {
    void init() {
        ptr.limit = sizeof(entries)-1;
        ptr.base = reinterpret_cast<uint32_t>(&entries);

        set_gate(0,0,0,0,0); //null descriptor (required by the CPU)
        set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // 0x08 kernel code 
        set_gate(2,0,0xFFFFFFFF, 0x92, 0xCF); //0x10 kernel data

        gdt_flush(reinterpret_cast<uint32_t>(&ptr));
    }
}


