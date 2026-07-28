

#include "idt.h"
#include "io.h"
#include "pic.h"
#include "terminal.h"
#include "serial.h"


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
        entries[num].base_high = (base >> 16) & 0xFFFF;
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

    void isr_handler(registers_t* regs) {

    if (regs->int_no == 14) {

        uint32_t fault_addr;
        asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
        terminal::set_color(COLOR_LIGHT_RED, COLOR_BLACK);

        terminal::write("idt PAGE FAULT at 0x");
        terminal::write_hex(fault_addr);

        terminal::write(" (err_code ");
        terminal::write_dec(regs->err_code);
        terminal::write(")\n");

        serial::write("idt PAGE FAULT caught");
        
        serial::write("\n");

        for (;;) asm volatile("hlt");
    }

    terminal::set_color(COLOR_LIGHT_RED, COLOR_BLACK);
    terminal::write("EXCEPTION: ");
    terminal::write(exception_name(regs->int_no));
    terminal::write("\n");

    for (;;) asm volatile("hlt");
}


    void irq_handler(registers_t* regs) {
        int irq = regs->int_no - 32;

        if (irq >= 0 && irq < 16 && irq_routines[irq]) {
            irq_routines[irq](regs);
        }

        pic::send_eoi(irq);
    }
}


namespace idt {

    void install_irq_handler(int irq, irq_handler_t handler) {
        if (irq >= 0 && irq < 16) {
            irq_routines[irq] = handler;
        }
    }

    void init() {
        ptr.limit = sizeof(entries) - 1;
        ptr.base = reinterpret_cast<uint32_t>(&entries);

        for (auto& e : entries) {
            e = {};
        }

        set_gate(0,  reinterpret_cast<uint32_t>(isr0),  0x08, 0x8E);
        set_gate(1,  reinterpret_cast<uint32_t>(isr1),  0x08, 0x8E);
        set_gate(2,  reinterpret_cast<uint32_t>(isr2),  0x08, 0x8E);
        set_gate(3,  reinterpret_cast<uint32_t>(isr3),  0x08, 0x8E);
        set_gate(4,  reinterpret_cast<uint32_t>(isr4),  0x08, 0x8E);
        set_gate(5,  reinterpret_cast<uint32_t>(isr5),  0x08, 0x8E);
        set_gate(6,  reinterpret_cast<uint32_t>(isr6),  0x08, 0x8E);
        set_gate(7,  reinterpret_cast<uint32_t>(isr7),  0x08, 0x8E);
        set_gate(8,  reinterpret_cast<uint32_t>(isr8),  0x08, 0x8E);
        set_gate(9,  reinterpret_cast<uint32_t>(isr9),  0x08, 0x8E);
        set_gate(10, reinterpret_cast<uint32_t>(isr10), 0x08, 0x8E);
        set_gate(11, reinterpret_cast<uint32_t>(isr11), 0x08, 0x8E);
        set_gate(12, reinterpret_cast<uint32_t>(isr12), 0x08, 0x8E);
        set_gate(13, reinterpret_cast<uint32_t>(isr13), 0x08, 0x8E);
        set_gate(14, reinterpret_cast<uint32_t>(isr14), 0x08, 0x8E);
        set_gate(15, reinterpret_cast<uint32_t>(isr15), 0x08, 0x8E);
        set_gate(16, reinterpret_cast<uint32_t>(isr16), 0x08, 0x8E);
        set_gate(17, reinterpret_cast<uint32_t>(isr17), 0x08, 0x8E);
        set_gate(18, reinterpret_cast<uint32_t>(isr18), 0x08, 0x8E);
        set_gate(19, reinterpret_cast<uint32_t>(isr19), 0x08, 0x8E);
        set_gate(20, reinterpret_cast<uint32_t>(isr20), 0x08, 0x8E);
        set_gate(21, reinterpret_cast<uint32_t>(isr21), 0x08, 0x8E);
        set_gate(22, reinterpret_cast<uint32_t>(isr22), 0x08, 0x8E);
        set_gate(23, reinterpret_cast<uint32_t>(isr23), 0x08, 0x8E);
        set_gate(24, reinterpret_cast<uint32_t>(isr24), 0x08, 0x8E);
        set_gate(25, reinterpret_cast<uint32_t>(isr25), 0x08, 0x8E);
        set_gate(26, reinterpret_cast<uint32_t>(isr26), 0x08, 0x8E);
        set_gate(27, reinterpret_cast<uint32_t>(isr27), 0x08, 0x8E);
        set_gate(28, reinterpret_cast<uint32_t>(isr28), 0x08, 0x8E);
        set_gate(29, reinterpret_cast<uint32_t>(isr29), 0x08, 0x8E);
        set_gate(30, reinterpret_cast<uint32_t>(isr30), 0x08, 0x8E);
        set_gate(31, reinterpret_cast<uint32_t>(isr31), 0x08, 0x8E);

        // IRQs remapped to vectors 32-47
        set_gate(32, reinterpret_cast<uint32_t>(irq0),  0x08, 0x8E);
        set_gate(33, reinterpret_cast<uint32_t>(irq1),  0x08, 0x8E);
        set_gate(34, reinterpret_cast<uint32_t>(irq2),  0x08, 0x8E);
        set_gate(35, reinterpret_cast<uint32_t>(irq3),  0x08, 0x8E);
        set_gate(36, reinterpret_cast<uint32_t>(irq4),  0x08, 0x8E);
        set_gate(37, reinterpret_cast<uint32_t>(irq5),  0x08, 0x8E);
        set_gate(38, reinterpret_cast<uint32_t>(irq6),  0x08, 0x8E);
        set_gate(39, reinterpret_cast<uint32_t>(irq7),  0x08, 0x8E);
        set_gate(40, reinterpret_cast<uint32_t>(irq8),  0x08, 0x8E);
        set_gate(41, reinterpret_cast<uint32_t>(irq9),  0x08, 0x8E);
        set_gate(42, reinterpret_cast<uint32_t>(irq10), 0x08, 0x8E);
        set_gate(43, reinterpret_cast<uint32_t>(irq11), 0x08, 0x8E);
        set_gate(44, reinterpret_cast<uint32_t>(irq12), 0x08, 0x8E);
        set_gate(45, reinterpret_cast<uint32_t>(irq13), 0x08, 0x8E);
        set_gate(46, reinterpret_cast<uint32_t>(irq14), 0x08, 0x8E);
        set_gate(47, reinterpret_cast<uint32_t>(irq15), 0x08, 0x8E);

        idt_flush(reinterpret_cast<uint32_t>(&ptr));
    }
}