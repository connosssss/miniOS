 
#include "pic.h"
#include "io.h"


namespace {
    constexpr uint16_t PIC1 = 0x20;
    constexpr uint16_t PIC1_DATA = 0x21;
    constexpr uint16_t PIC2 = 0xA0;
    constexpr uint16_t PIC2_DATA = 0xA1;
    constexpr uint8_t ICW1_INIT = 0x10;
    constexpr uint8_t ICW1_ICW4 = 0x01;
    constexpr uint8_t ICW4_8086 = 0x01;
}


namespace pic {
    void remap() {
        //begin initialization sequence on both PICs 
        outb(PIC1, ICW1_INIT | ICW1_ICW4);
        io_wait();
        outb(PIC2, ICW1_INIT | ICW1_ICW4);
        io_wait();

        // ICW2 / vector offsets.
        //  IRQ0-7 -> 32-39, IRQ8-15 -> 40-47.
        outb(PIC1_DATA, 0x20);
        io_wait();
        outb(PIC2_DATA, 0x28);
        io_wait();

        // ICW3 / telling each PIC how theyre cascaded together
        outb(PIC1_DATA, 0x04); 
        io_wait();
        outb(PIC2_DATA, 0x02); 
        io_wait();

        // ICW4 -> 8086/88 mode.
        outb(PIC1_DATA, ICW4_8086);
        io_wait();
        outb(PIC2_DATA, ICW4_8086);
        io_wait();



        // Masks everything except for IRQ1 for now
        // |-> Will need to change in the future 
        outb(PIC1_DATA, 0xFD);
        outb(PIC2_DATA, 0xFF);

    }

    void send_eoi(int irq) {
        if(irq >= 8) outb(PIC2, 0x20);
        outb(PIC1, 0x20);
    }
}