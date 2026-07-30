#include "syscall.h"
#include "syscall_abi.h"
#include "terminal.h"
#include "serial.h"
#include "keyboard.h"
#include "vfs.h"

#include "io.h"

namespace {
    volatile uint64_t timer_ticks = 0;

    void timer_irq_handler(registers_t*) {
        timer_ticks++;
    }
}

namespace sys {

    void init_timer() {
        idt::install_irq_handler(0, timer_irq_handler);
        uint32_t divisor = 1193; // 1193182 / 1193 = ~1000 Hz (1 ms per tick)
        outb(0x43, 0x36);
        outb(0x40, divisor & 0xFF);
        outb(0x40, (divisor >> 8) & 0xFF);
    }

    void dispatch(registers_t* regs) {
        asm volatile("sti"); // Re-enable interrupts inside interrupt handler

        switch (regs->eax) {

            case SYS_WRITE: {
                const char* str = reinterpret_cast<const char*>(regs->ebx);
                terminal::write(str);
                serial::write(str);
                regs->eax = 0;
                break;
            }

            case SYS_GETCHAR: {
                char c;

                while (!keyboard::try_read(&c)) {
                    asm volatile("hlt");
                }

                regs->eax = static_cast<uint32_t>(c);
                break;
            }

            case SYS_READ_FILE: {
                const char* path = reinterpret_cast<const char*>(regs->ebx);
                
                char* buf        = reinterpret_cast<char*>(regs->ecx);
                uint32_t size    = regs->edx;

                regs->eax = vfs::read_file(path, buf, size);
                break;
            }

            case SYS_LIST_FILES: {
                char* buf     = reinterpret_cast<char*>(regs->ebx);
                uint32_t size = regs->ecx;

                regs->eax = vfs::list_files(buf, size);
                break;
            }

            case SYS_CLEAR: {
                terminal::clear();
                serial::write("\033[2J\033[H");
                regs->eax = 0;
                break;
            }

            case SYS_CREATE_FILE: {
                const char* path = reinterpret_cast<const char*>(regs->ebx);
                const char* content = reinterpret_cast<const char*>(regs->ecx);
                uint32_t len = regs->edx;
                regs->eax = vfs::create_file(path, content, len);
                break;
            }

            case SYS_POLLCHAR: {
                char c = 0;
                if (keyboard::try_read(&c)) {
                    regs->eax = static_cast<uint32_t>(c);
                } 
                else {
                    regs->eax = 0;
                }
                break;
            }

            case SYS_SLEEP: {
                uint32_t ms = regs->ebx;
                uint64_t target = timer_ticks + ms;
                while (timer_ticks < target) {
                    asm volatile("hlt");
                }
                regs->eax = 0;
                break;
            }

            case SYS_SET_COLOR: {
                VgaColor fg = static_cast<VgaColor>(regs->ebx);
                VgaColor bg = static_cast<VgaColor>(regs->ecx);
                terminal::set_color(fg, bg);
                regs->eax = 0;
                break;
            }

            case SYS_DELETE_FILE: {
                const char* path = reinterpret_cast<const char*>(regs->ebx);
                regs->eax = vfs::delete_file(path);
                break;
            }

            default:
                regs->eax = static_cast<uint32_t>(-1);
                break;

        }
    }
}
