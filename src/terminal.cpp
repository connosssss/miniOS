#include "terminal.h"
#include <stddef.h>
#include "io.h"

namespace terminal {

    namespace {
        constexpr size_t WIDTH  = 80;
        constexpr size_t HEIGHT = 25;

        size_t row {};
        size_t col {};
        uint8_t color {0};
        uint16_t* buffer {nullptr};

        uint8_t to_color(VgaColor fg, VgaColor bg) {
            return fg | (bg << 4);
        }

        uint16_t make_vga_entry(char c, uint8_t clr) {
            return static_cast<uint16_t>(c) | (static_cast<uint16_t>(clr) << 8);
        }

        void clear() {
            for (size_t y = 0; y < HEIGHT; y++)
                for (size_t x = 0; x < WIDTH; x++)
                    buffer[y * WIDTH + x] = make_vga_entry(' ', color);
        }

        void scroll() {
            for (size_t y = 1; y < HEIGHT; y++)
                for (size_t x = 0; x < WIDTH; x++)
                    buffer[(y - 1) * WIDTH + x] = buffer[y * WIDTH + x];

            for (size_t x = 0; x < WIDTH; x++)
                buffer[(HEIGHT - 1) * WIDTH + x] = make_vga_entry(' ', color);

            row = HEIGHT - 1;
        }
    }

    void init() {
        color = to_color(COLOR_WHITE, COLOR_BLACK);
        buffer = reinterpret_cast<uint16_t*>(0xB8000);
        clear();
    }

    void update_cursor() {
        uint16_t pos = row * WIDTH + col;
        
        outb(0x3D4, 0x0F);
        outb(0x3D5, pos & 0xFF);
        outb(0x3D4, 0x0E);
        outb(0x3D5, (pos >> 8) & 0xFF);
    }

    void write_char(char c) {
        if (c == '\n') {
            col = 0;
            if (++row == HEIGHT) scroll();
            return;
        }

        const size_t index = row * WIDTH + col;
        buffer[index] = make_vga_entry(c, color);

        if (++col == WIDTH) {
            col = 0;
            if (++row == HEIGHT) scroll();
        }

        update_cursor();
    }

    void write(const char* data) {
        for (size_t i = 0; data[i] != '\0'; i++)
            write_char(data[i]);
    }

    void set_color(VgaColor fg, VgaColor bg) {
        color = to_color(fg, bg);
    }

}
