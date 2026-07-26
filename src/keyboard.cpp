
#include "keyboard.h"
#include "idt.h"
#include "io.h"
#include <stdint.h>
#include "terminal.h"

namespace {
    // Unshifted US QWERTY, scancode set 1.
    // The index is the scancode and value is ascii
    // (0 = no printable mapping for the current driver like function keys, ctrl,
    // alt, caps lock, arrow keys, etc.)
    const char scancode_ascii[128] = {
        0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0,   '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0,   ' ',
        // remaining entries stay 0 for now 
    };



    void handle_irq(registers_t*) {
        uint8_t scancode = inb(0x60);

        if (scancode & 0x80) return;

        char c = (scancode < 128) ? scancode_ascii[scancode] : 0;
        if (c) terminal::write_char(c);
    }
    
}

namespace keyboard {

    void init() {
        idt::install_irq_handler(1, handle_irq);
    }

}
