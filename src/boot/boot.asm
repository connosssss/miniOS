MBALIGN equ 1<<0
MEMINFO equ 1<<1
FLAGS   equ MBALIGN | MEMINFO
MAGIC   equ 0x1BADB002         ; Number grub looks for
CHECKSUM  equ -(MAGIC + FLAGS)



section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM


; Building a useable stack in .bss
section .bss
align 16
global stack_top
stack_bottom:
    resb 16384    ; 16 kb kernel stack 
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits




section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    push ebx
    push eax
    call kernel_main
    cli

.hang:
    hlt
    jmp .hang
