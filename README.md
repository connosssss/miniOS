# miniOS

A small 32-bit x86 kernel written from scratch in C++ and NASM assembly, built as a learning project for OS development fundamentals: booting, segmentation, paging, interrupts, multitasking, filesystems, system calls, and user space.

It boots via the Multiboot specification, runs directly in QEMU, and brings up a preemptive kernel with an interactive Ring 3 shell, disk-backed filesystem, live status bar, and basic user applications.

## Features

- **Multiboot-compliant boot** — `boot.asm` sets up the Multiboot header, a 16 KB kernel stack, and hands off to `kernel_main` in C++
- **GDT & TSS** — flat memory model with kernel/user segments and TSS for privilege switching
- **IDT & PIC remapping** — 32 CPU exception handlers, remapped 8259 PIC IRQs (32–47), and `int 0x80` syscall gate
- **Memory management** — bitmap Physical Memory Manager (PMM), 2-tier virtual memory paging, per-process address spaces, and dynamic kernel heap (`kmalloc`/`kfree`)
- **Preemptive multitasking** — round-robin scheduler with task state management (`RUNNABLE`, `SLEEPING`, `ZOMBIE`)
- **User space & System calls** — Ring 3 execution with an `int 0x80` system call interface for I/O, file management, process info, and sleep
- **Filesystems & Disk driver** — ATA PIO disk driver, custom persistent on-disk filesystem (`miniFS`), VFS abstraction, and initrd RAMDisk
- **Ring 3 Shell & Apps** — interactive shell (`help`, `ps`, `ls`, `cat`, `touch`, `rm`, `color`, `clear`), live top status bar UI, background system monitor, and Snake game
- **VGA terminal & Keyboard driver** — 80x25 text-mode console with 16-color support and PS/2 keyboard scancode decoder
- **Serial output** — COM1 serial driver for debug logs and system telemetry

## Prerequisites

- `g++` with 32-bit support (`-m32`), C++17
- `nasm`
- `ld` (GNU binutils, with `elf_i386` support)
- `python3` (for initrd generation)
- `qemu-system-i386` (for running the kernel)

On Debian/Ubuntu:
```bash
sudo apt install build-essential g++-multilib nasm qemu-system-x86 python3
```

On Arch:
```bash
sudo pacman -S base-devel nasm qemu-system-x86 lib32-gcc-libs python
```


## Building and running

```bash
make 
make run    
make clean  
```

When it boots, QEMU will launch the kernel alongside the RAM disk and disk image. You will see boot diagnostics on VGA and serial output, entering an interactive Ring 3 shell with a live status bar.
