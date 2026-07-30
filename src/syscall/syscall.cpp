#include "syscall.h"
#include "syscall_abi.h"
#include "terminal.h"
#include "serial.h"
#include "keyboard.h"
#include "vfs.h"
#include "process.h"

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

    uint64_t get_ticks() {
        return timer_ticks;
    }

    void dispatch(registers_t* regs) {
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
                    asm volatile("sti; hlt");
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
                process::sleep_current(ms, timer_ticks);
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

            case SYS_YIELD: {
                regs->eax = 0;
                break;
            }

            case SYS_GET_TICKS: {
                regs->eax = static_cast<uint32_t>(timer_ticks);
                break;
            }

            case SYS_GET_PROC_COUNT: {
                regs->eax = process::count();
                break;
            }

            case SYS_WRITE_AT: {
                // ebx = (row << 16 | col), ecx = str pointer, edx = color_attr
                uint16_t r = static_cast<uint16_t>(regs->ebx >> 16);
                uint16_t c = static_cast<uint16_t>(regs->ebx & 0xFFFF);

                const char* str = reinterpret_cast<const char*>(regs->ecx);
                uint8_t color_attr = static_cast<uint8_t>(regs->edx);
                terminal::write_at(r, c, str, color_attr);


                regs->eax = 0;
                break;
            }

            case SYS_WRITE_SERIAL: {
                const char* str = reinterpret_cast<const char*>(regs->ebx);
                serial::write(str);
                regs->eax = 0;

                break;
            }

            case SYS_LIST_PROCS: {
                char* buf = reinterpret_cast<char*>(regs->ebx);
                uint32_t size = regs->ecx;
                regs->eax = process::list_procs(buf, size);
                
                break;
            }

            default:
                regs->eax = static_cast<uint32_t>(-1);
                break;

        }
    }
}
