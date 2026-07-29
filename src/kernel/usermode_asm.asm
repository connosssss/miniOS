global jump_to_usermode
jump_to_usermode:
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]

    mov ax, 0x23 ; User data segment selector (0x20 | RPL 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword 0x23 
    push ecx 
    pushfd
    or dword [esp], 0x200 
    push dword 0x1B 
    push ebx
    iret
