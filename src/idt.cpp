

#include "idt.h"


namespace {
    struct idt_entry_t {
        uint16_t base_low;
        uint16_t sel;
        uint8_t  always0 {};
        uint8_t  flags;
        uint16_t base_high;
    } __attribute__((packed));

    struct idt_ptr_t {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    idt_entry_t entries[256];
    idt_ptr_t   ptr;
    irq_handler_t irq_routines[16] = { nullptr };


    void set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
        entries[num].base_low = base & 0xFFFF;
        entires[num].base_high = (base >> 16) & 0xFFFF;
        entries[num].sel = sel;
        entries[num].flags = flags;
    }

    const char* exception_name(uint32_t n) {
        static const char* names[32] = {
            "Division By Zero",
             "Debug", 
             "Non-Maskable Interrupt", 
             "Breakpoint",
            "Overflow", 
            "Bound Range Exceeded", 
            "Invalid Opcode",
            "Device Not Available", 
            "Double Fault", 
            "Coprocessor Segment Overrun",
            "Invalid TSS", 
            "Segment Not Present", 
            "Stack-Segment Fault",
            "General Protection Fault", 
            "Page Fault", 
            "Reserved",
            "x87 Floating-Point Exception", 
            "Alignment Check", 
            "Machine Check",
            "SIMD Floating-Point Exception", 
            "Virtualization Exception",
            "Control Protection Exception", 
            "Reserved", 
            "Reserved", 
            "Reserved",
            "Reserved", 
            "Reserved", 
            "Reserved", 
            "Hypervisor Injection Exception",
            "VMM Communication Exception", 
            "Security Exception", 
            "Reserved"
        };

        return (n < 32) ? names[n] : "Unknown";
    }


    extern "C" {
        void isr0();  void isr1();  void isr2();  void isr3();  void isr4();
        void isr5();  void isr6();  void isr7();  void isr8();  void isr9();
        void isr10(); void isr11(); void isr12(); void isr13(); void isr14();
        void isr15(); void isr16(); void isr17(); void isr18(); void isr19();
        void isr20(); void isr21(); void isr22(); void isr23(); void isr24();
        void isr25(); void isr26(); void isr27(); void isr28(); void isr29();
        void isr30(); void isr31();
        
        void irq0();  void irq1();  void irq2();  void irq3();  void irq4();
        void irq5();  void irq6();  void irq7();  void irq8();  void irq9();
        void irq10(); void irq11(); void irq12(); void irq13(); void irq14();
        void irq15();
        
        void idt_flush(uint32_t idt_ptr_addr);
    }
}