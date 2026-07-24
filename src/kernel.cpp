#include <stdint.h>
#include<stddef.h>

#include "gdt.h"




enum VgaColor {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15,
};






// Serial port / COM1 -> first serial port on an x86 computer
// will allow us to see kernel output without a display server or graphics driver
namespace serial {

    //COM1 PORT
    constexpr uint16_t PORT {0x3F8};


    // write a byte to an x86 IO port
    static inline void outb(uint16_t port, uint8_t val) { 
        asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port)); //volatile to tell compiler to not optimize

    }
    
    // read a byte
    static inline uint8_t inb(uint16_t port) {
        uint8_t res;
        asm volatile("inb %1, %0" : "=a" (res) : "Nd"(port));

        return res;
    }


    void init() {
        outb(PORT+1, 0x00);
        outb(PORT + 3, 0x80);
        outb(PORT + 0, 0x03);
        outb(PORT + 1, 0x00);
        outb(PORT + 3, 0x03);
        outb(PORT + 2, 0xC7);
        outb(PORT + 4, 0x0B);
    }

    bool is_transmit_ready() {
        return inb(PORT + 5) & 0x20;
    }

    void write_char(const char& c){
        while(!is_transmit_ready())  { ; } //pause
        outb(PORT, static_cast<uint8_t>(c));
    }

    void write(const char* str){
        for(size_t i = 0; str[i] != '\0'; ++i)
            write_char(str[i]);
    }
    


}



class Terminal {

    static constexpr size_t WIDTH {80};
    static constexpr size_t HEIGHT {25};

    size_t row {}; size_t col {};
    uint8_t color {0};
    uint16_t* buffer {nullptr};

    uint8_t to_color(VgaColor fg, VgaColor bg){
        return fg | (bg << 4);
    }

    uint16_t make_vga_entry(const char& c, const uint8_t& color){
        return static_cast<uint16_t>(c) | (static_cast<uint16_t>(color) << 8);
    }



    public:

    
    void init() {
        color = to_color(COLOR_WHITE, COLOR_BLACK);
        buffer = reinterpret_cast<uint16_t*>(0xB8000);
        clear();
    }

    void clear() {

        for (size_t y = 0; y < HEIGHT; y++) {
            for (size_t x = 0; x < WIDTH; x++) {

                const size_t index = y * WIDTH + x;
                buffer[index] = make_vga_entry(' ', color);

            }
        }
    }

    void put_char(char c) {
        if (c == '\n') {
            col = 0;
            if (++row == HEIGHT) {
                scroll();
            }
            return;
        }

        const size_t index = row * WIDTH + col;
        buffer[index] = make_vga_entry(c, color);
        
        if (++col == WIDTH) {
            col = 0;
            if (++row == HEIGHT) {
                scroll();
            }
        }
    }

    void scroll() {

        for (size_t y = 1; y < HEIGHT; y++) {
            for (size_t x = 0; x < WIDTH; x++) {
                buffer[(y - 1) * WIDTH + x] = buffer[y * WIDTH + x];
            }
        }

        for (size_t x = 0; x < WIDTH; x++) {
            buffer[(HEIGHT - 1) * WIDTH + x] = make_vga_entry(' ', color);
        }

        row = HEIGHT - 1;
    }

    void write(const char* data) {
        for (size_t i = 0; data[i] != '\0'; i++) {
            put_char(data[i]);
        }
    }


    void set_color(const VgaColor& fg, const VgaColor& bg) {
        color = to_color(fg, bg);
    }
};

Terminal terminal;







extern "C" void kernel_main(){




    serial::init();
    terminal.init();

    // might change to auto add new line char
    serial::write("aoeuoeuoaeu \n"); 
    terminal.write("OS booted \n");
    terminal.write("Helloworld.\n");

    gdt::init();
    terminal.set_color(COLOR_LIGHT_CYAN, COLOR_BLACK);
    terminal.write("GDT kernel code and data segments installed\n");
}