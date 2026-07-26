#include <stdint.h>
#include<stddef.h>

#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "terminal.h"
#include "serial.h"


extern "C" void kernel_main(){




    serial::init();
    terminal::init();

    terminal::write("OS booted \n");
    serial::write("OS booted \n");

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