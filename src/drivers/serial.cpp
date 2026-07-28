#include "serial.h"
#include "io.h"
#include <stdint.h>
#include <stddef.h>

// Serial port / COM1 -> first serial port on an x86 computer
// will allow us to see kernel output without a display server or graphics driver
namespace serial {

    //COM1 PORT
    constexpr uint16_t PORT {0x3F8};

    void init() {
        outb(PORT + 1, 0x00);
        outb(PORT + 3, 0x80);
        outb(PORT + 0, 0x03);
        outb(PORT + 1, 0x00);
        outb(PORT + 3, 0x03);
        outb(PORT + 2, 0xC7);
        outb(PORT + 4, 0x0B);
    }

    static bool is_transmit_ready() {
        return inb(PORT + 5) & 0x20;
    }

    void write_char(char c) {
        while (!is_transmit_ready()) { ; } //pause
        outb(PORT, static_cast<uint8_t>(c));
    }

    void write(const char* str) {
        for (size_t i = 0; str[i] != '\0'; ++i)
            write_char(str[i]);
    }
}
