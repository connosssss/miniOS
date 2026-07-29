
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

    constexpr uint32_t BUFFER_SIZE = 256;
    char ring_buffer[BUFFER_SIZE];
    uint32_t head = 0;
    uint32_t tail = 0;


    void handle_irq(registers_t*) {
        uint8_t scancode = inb(0x60);

        if (scancode & 0x80) return;

        char c = (scancode < 128) ? scancode_ascii[scancode] : 0;

        if (c) {
            uint32_t next = (head + 1) % BUFFER_SIZE;
            if (next != tail) {
                ring_buffer[head] = c;
                head = next;
            }
        }

    }
    
}

namespace keyboard {

    void init() {
        idt::install_irq_handler(1, handle_irq);
    }

    bool try_read(char* out_char) {
        if (head == tail) return false;

        *out_char = ring_buffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;
        return true;
    }

}
