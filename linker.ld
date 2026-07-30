ENTRY(_start)

SECTIONS
{
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(EXCLUDE_FILE(*shell.o *statusbar.o *sysmon.o) .text)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(EXCLUDE_FILE(*shell.o *statusbar.o *sysmon.o) .rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(EXCLUDE_FILE(*shell.o *statusbar.o *sysmon.o) .data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(EXCLUDE_FILE(*shell.o *statusbar.o *sysmon.o) COMMON)
		*(EXCLUDE_FILE(*shell.o *statusbar.o *sysmon.o) .bss)
	}

	_user_start = .;
	.user_text BLOCK(4K) : ALIGN(4K)
	{
		*shell.o(.text .text.*)
		*statusbar.o(.text .text.*)
		*sysmon.o(.text .text.*)
	}
	.user_rodata BLOCK(4K) : ALIGN(4K)
	{
		*shell.o(.rodata .rodata.*)
		*statusbar.o(.rodata .rodata.*)
		*sysmon.o(.rodata .rodata.*)
	}
	.user_data BLOCK(4K) : ALIGN(4K)
	{
		*shell.o(.data .data.*)
		*statusbar.o(.data .data.*)
		*sysmon.o(.data .data.*)
	}
	.user_bss BLOCK(4K) : ALIGN(4K)
	{
		*shell.o(COMMON .bss .bss.*)
		*statusbar.o(COMMON .bss .bss.*)
		*sysmon.o(COMMON .bss .bss.*)
	}
	_user_end = .;

	_kernel_end = .;
}
