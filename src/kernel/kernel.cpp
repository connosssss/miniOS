#include <stdint.h>
#include<stddef.h>

#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "terminal.h"
#include "serial.h"
#include "pmm.h"
#include "heap.h"


extern "C" void kernel_main(uint32_t magic, uint32_t multiboot_info_addr){




    serial::init();
    terminal::init();

    terminal::write("OS booted \n");
    serial::write("OS booted \n");

    pmm::init(multiboot_info_addr);
    heap::init();

    //testing heap 
    /*
    void* ptr1 = heap::kmalloc(100);
    terminal::write("Allocating ptr1 on heap (100 bytes): 0x");
    terminal::write_hex(reinterpret_cast<uint32_t>(ptr1));
    terminal::write("\n");

    void* ptr2 = heap::kmalloc(200);
    terminal::write("Allocating ptr2 on heap (200 bytes): 0x");
    terminal::write_hex(reinterpret_cast<uint32_t>(ptr2));
    terminal::write("\n");

    terminal::write("Freeing ptr1\n");
    heap::kfree(ptr1);*/

    gdt::init();
    terminal::set_color(COLOR_LIGHT_CYAN, COLOR_BLACK);
    terminal::write("GDT kernel code and data segments installed\n");
    serial::write("GDT kernel code and data segments installed\n");
    

    idt::init();
    terminal::write("IDT interupt table installed\n");
    serial::write("IDT interupt table installed\n");

    pic::remap();

    keyboard::init();
    asm volatile("sti");  // enable interrupts
    terminal::set_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
    terminal::write("IRQ1 handler registered and interrupts enabled\n");
    serial::write("IRQ1 handler registered and interrupts enabled\n");

    terminal::set_color(COLOR_WHITE, COLOR_BLACK);
    terminal::write("\nType on the keyboard:\n");
    serial::write("\nType on the keyboard:\n");

    // Halt and wait for interrupts forever
    for (;;) asm volatile("hlt");
}