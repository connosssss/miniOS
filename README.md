# miniOS

A small 32-bit x86 kernel written from scratch in C++ and NASM assembly, built as a
learning project for OS development fundamentals: booting, segmentation, interrupts,
and basic I/O.

It boots via the Multiboot specification, runs directly in QEMU, and currently brings
up a VGA text-mode terminal, a serial (COM1) debug console, and a PS/2 keyboard driver.

## Features

- **Multiboot-compliant boot** — `boot.asm` sets up the Multiboot header, a 16 KB
  kernel stack, and hands off to `kernel_main` in C++
- **GDT** — flat memory model with kernel code/data segments
- **IDT** — all 32 CPU exception vectors (with human-readable names on fault) and
  16 remapped hardware IRQ vectors
- **PIC remapping** — legacy 8259 PIC remapped so IRQs 0–15 land on interrupt
  vectors 32–47, clear of the CPU exception range
- **Keyboard driver** — IRQ1 handler decoding scancode set 1 (unshifted US QWERTY)
- **VGA text-mode terminal** — 80x25 buffer with scrolling, cursor updates, and
  16-color foreground/background support
- **Serial output** — COM1 driver so kernel logs are visible without a display
  (handy for headless QEMU or debugging)


## Prerequisites
- `g++` with 32-bit support (`-m32`), C++17
- `nasm`
- `ld` (GNU binutils, with `elf_i386` support)
- `qemu-system-i386` (for running the kernel)

On Debian/Ubuntu:
```bash
sudo apt install build-essential g++-multilib nasm qemu-system-x86
```

On Arch:
```bash
sudo pacman -S base-devel nasm qemu-system-x86 lib32-gcc-libs
```


## Building and running

```bash
make 
make run    
make clean  
```

When it boots, you should see boot progress messages on both the VGA screen and
the serial console, interrupts get enabled, and keyboard input is echoed to the screen
