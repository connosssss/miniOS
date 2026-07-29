#include "syscall.h"
#include "syscall_abi.h"
#include "terminal.h"
#include "serial.h"
#include "keyboard.h"
#include "vfs.h"

namespace sys {


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

            default:
                regs->eax = static_cast<uint32_t>(-1);
                break;
        }
    }
}
