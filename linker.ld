ENTRY(_start)

SECTIONS
{
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(EXCLUDE_FILE(*shell.o) .text)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(EXCLUDE_FILE(*shell.o) .rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(EXCLUDE_FILE(*shell.o) .data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(EXCLUDE_FILE(*shell.o) COMMON)
		*(EXCLUDE_FILE(*shell.o) .bss)
	}

	_user_start = .;
	.user_text BLOCK(4K) : ALIGN(4K)
	{
		*shell.o(.text .text.*)
	}
	.user_rodata BLOCK(4K) : ALIGN(4K)
	{
		*shell.o(.rodata .rodata.*)
	}
	.user_data BLOCK(4K) : ALIGN(4K)
	{
		*shell.o(.data .data.*)
	}
	.user_bss BLOCK(4K) : ALIGN(4K)
	{
		*shell.o(COMMON .bss .bss.*)
	}
	_user_end = .;

	_kernel_end = .;
}
