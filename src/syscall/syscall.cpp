#include "syscall.h"
#include "syscall_abi.h"
#include "terminal.h"
#include "keyboard.h"

namespace sys {


    void dispatch(registers_t* regs) {
        asm volatile("sti"); // Re-enable interrupts inside interrupt handler

        switch (regs->eax) {

            case SYS_WRITE: {
                const char* str = reinterpret_cast<const char*>(regs->ebx);
                terminal::write(str);
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

    /*      case SYS_READ_FILE: {
                
            }

            case SYS_LIST_FILES: {

            } */

            default:
                regs->eax = static_cast<uint32_t>(-1);
                break;
        }
    }
}
