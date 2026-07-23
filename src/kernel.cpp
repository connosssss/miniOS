#include <stdint.h>
#include<stddef.h>


extern "C" void kernel_main();









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
        asm volatile("inb %1, %0" : "=a" (ret) : "Nd"(port));

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