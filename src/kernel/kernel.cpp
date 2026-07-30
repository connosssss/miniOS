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
#include "ata.h"
#include "minifs.h"

#include "process.h"

extern "C" char _user_start[];
extern "C" char _user_end[];
extern "C" char stack_top[];

extern "C" void shell_main();
extern "C" void jump_to_usermode(uint32_t entry_point, uint32_t user_stack);
extern "C" void statusbar_main();
extern "C" void sysmon_main();

namespace {
    void background_task() {
        uint32_t ticks = 0;

        while (1) {
            ticks++;

            if (ticks % 10000000 == 0) {
                serial::write("background Task: concurrently running task\n");
            }
            
            process::yield();
        }
    }
}


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

    // Initialize disk-backed filesystem
    if (ata::init()) {
        minifs::init();
    }

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

    process::init();
    process_t* shell_proc = process::create(shell_main, true, "shell");
    process_t* bar_proc = process::create(statusbar_main, true, "statusbar");
    process_t* mon_proc = process::create(sysmon_main, true, "sysmon");

    process_t* bg_proc = process::create(background_task, false, "background");

    terminal::set_color(COLOR_LIGHT_CYAN, COLOR_BLACK);
    terminal::write("multitasking init -> ");
    terminal::write_dec(process::count());
    terminal::write(" tasks (shell=PID");
    terminal::write_dec(shell_proc->pid);
    terminal::write(" bar=PID");
    terminal::write_dec(bar_proc->pid);
    terminal::write(" mon=PID");
    terminal::write_dec(mon_proc->pid);
    terminal::write(" bg=PID");
    terminal::write_dec(bg_proc->pid);
    terminal::write(")\n");

    serial::write("multitasking init\n");
    process::start_multitasking();

    for (;;) asm volatile("hlt");
}