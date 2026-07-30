#include "terminal.h"
#include "kutil.h"
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

        // Row 0 is reserved for the status bar
        constexpr size_t FIRST_ROW = 1;

        void scroll() {
            for (size_t y = FIRST_ROW + 1; y < HEIGHT; y++)
                for (size_t x = 0; x < WIDTH; x++)
                    buffer[(y - 1) * WIDTH + x] = buffer[y * WIDTH + x];

            for (size_t x = 0; x < WIDTH; x++)
                buffer[(HEIGHT - 1) * WIDTH + x] = make_vga_entry(' ', color);

            row = HEIGHT - 1;
        }
    }

    void update_cursor();

    void clear() {
        for (size_t y = FIRST_ROW; y < HEIGHT; y++)
            for (size_t x = 0; x < WIDTH; x++)
                buffer[y * WIDTH + x] = make_vga_entry(' ', color);
        row = FIRST_ROW;
        col = 0;
        update_cursor();
    }

    void init() {
        color = to_color(COLOR_WHITE, COLOR_BLACK);
        buffer = reinterpret_cast<uint16_t*>(0xB8000);
        for (size_t y = 0; y < HEIGHT; y++)
            for (size_t x = 0; x < WIDTH; x++)
                buffer[y * WIDTH + x] = make_vga_entry(' ', color);
            
        row = FIRST_ROW;
        col = 0;
        update_cursor();
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

        if (c == '\b') {
            if (col > 0) {
                col--;
            } else if (row > 0) {
                row--;
                col = WIDTH - 1;
            }
            update_cursor();
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

    void write_hex(unsigned int value) {
        char buf[11];
        kutil::hex_to_str(value, buf);
        write(buf);
    }

    void write_dec(unsigned int value) {
        char buf[11];
        kutil::dec_to_str(value, buf);
        write(buf);
    }

    void write_at(uint16_t r, uint16_t c, const char* str, uint8_t color_attr) {
        for (size_t i = 0; str[i] != '\0' && c + i < WIDTH; i++) {
            buffer[r * WIDTH + c + i] = make_vga_entry(str[i], color_attr);
        }
    }

}
