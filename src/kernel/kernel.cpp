#include <stdint.h>
#include <stddef.h>

#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "terminal.h"
#include "serial.h"
#include "pmm.h"
#include "heap.h"
#include "vfs.h"
#include "paging.h"
#include "syscall.h"

extern "C" char _user_start[];
extern "C" char _user_end[];
extern "C" char stack_top[];

extern "C" void shell_main();
extern "C" void jump_to_usermode(uint32_t entry_point, uint32_t user_stack);

extern "C" void kernel_main(uint32_t magic, uint32_t multiboot_info_addr){
    (void)magic;

    serial::init();
    terminal::init();
    terminal::set_color(COLOR_LIGHT_MAGENTA, COLOR_BLACK);

    terminal::write("OS booted\n");
    serial::write("OS booted\n");

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
    gdt::set_kernel_stack(reinterpret_cast<uint32_t>(&stack_top));

    terminal::set_color(COLOR_LIGHT_CYAN, COLOR_BLACK);
    terminal::write("GDT kernel/user code and data segments and TSS installed\n");
    serial::write("GDT kernel/user code and data segments and TSS installed\n");

    idt::init();
    terminal::write("IDT interupt table installed (incl. int 0x80 syscall gate)\n");
    serial::write("IDT interupt table installed (incl. int 0x80 syscall gate)\n");

    pic::remap();

    keyboard::init();
    sys::init_timer();
    asm volatile("sti");  // enable interrupts
    terminal::set_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
    terminal::write("IRQ1 handler registered and interrupts enabled\n");
    serial::write("IRQ1 handler registered and interrupts enabled\n");

    paging::init();

    uint32_t user_size = reinterpret_cast<uint32_t>(_user_end) - reinterpret_cast<uint32_t>(_user_start);
    paging::set_user_accessible(reinterpret_cast<uint32_t>(_user_start), user_size);

    uint32_t user_stack_buf = reinterpret_cast<uint32_t>(heap::kmalloc(4096));
    paging::set_user_accessible(user_stack_buf, 4096);
    uint32_t user_stack_top = user_stack_buf + 4096;

    terminal::set_color(COLOR_WHITE, COLOR_BLACK);
    terminal::write("\nAll self-tests passed. Jumping to ring 3...\n\n");
    serial::write("\nAll self-tests passed. Jumping to ring 3...\n\n");

    jump_to_usermode(reinterpret_cast<uint32_t>(shell_main), user_stack_top);

    for (;;) asm volatile("hlt");
}