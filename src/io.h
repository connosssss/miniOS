#pragma once
#include <stdint.h>


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


static inline void io_wait() {
    outb(0x80, 0);
}