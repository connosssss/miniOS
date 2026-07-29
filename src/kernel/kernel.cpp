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
#include "vfs.h"


extern "C" void kernel_main(uint32_t magic, uint32_t multiboot_info_addr){




    serial::init();
    terminal::init();

    terminal::write("OS booted \n");
    serial::write("OS booted \n");

    pmm::init(multiboot_info_addr);
    vfs::init(multiboot_info_addr);
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

    



    char list_buf[256] = {0};
    int32_t bytes = vfs::list_files(list_buf, sizeof(list_buf));

    if (bytes > 0) {
        terminal::write("vfs Found files:\n");
        terminal::write(list_buf);
        serial::write("vfs Found files:\n");
        serial::write(list_buf);
    } 
    else {
        terminal::write("vfs error: No files found in initrd!\n");
        serial::write("vfs error: No files found in initrd!\n");
    }

    char file_buf[256] = {0};
    int32_t read_bytes = vfs::read_file("welcome.txt", file_buf, sizeof(file_buf) - 1);
    
    if (read_bytes >= 0) {
        file_buf[read_bytes] = '\0';
        terminal::write("vfs Contents of welcome.txt:\n");
        terminal::write(file_buf);
        terminal::write("\n");

        serial::write("vfs Contents of welcome.txt:\n");
        serial::write(file_buf);
        serial::write("\n");
    } 

    else {
        terminal::write("vfs error: Could not read welcome.txt\n");
        serial::write("vfs error: Could not read welcome.txt\n");
    }


    terminal::set_color(COLOR_WHITE, COLOR_BLACK);
    terminal::write("\nType on the keyboard:\n");
    serial::write("\nType on the keyboard:\n");
    
    // Halt and wait for interrupts forever
    for (;;) asm volatile("hlt");
}