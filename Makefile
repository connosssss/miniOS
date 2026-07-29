AS = nasm
CXX = g++
LD = ld

INCLUDE_DIRS = -Isrc -Isrc/boot -Isrc/gdt -Isrc/idt -Isrc/pic -Isrc/drivers -Isrc/memory -Isrc/utils -Isrc/kernel -Isrc/memory/paging -Isrc/filesystem -Isrc/syscall

ASFLAGS = -f elf32
CXXFLAGS = -m32 -std=c++17 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-pic -fno-pie -mno-sse -mno-sse2 -mno-mmx -nostdlib -Wall -Wextra $(INCLUDE_DIRS)
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

OBJ_DIR := obj

CPP_SOURCES := $(shell find src -type f -name '*.cpp')
ASM_SOURCES := $(shell find src -type f -name '*.asm')
OBJECTS     := $(CPP_SOURCES:src/%.cpp=$(OBJ_DIR)/%.o) $(ASM_SOURCES:src/%.asm=$(OBJ_DIR)/%.o)

.PHONY: all run clean

all: myos.bin initrd.img

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p "$(dir $@)"
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/%.o: src/%.asm
	@mkdir -p "$(dir $@)"
	$(AS) $(ASFLAGS) "$<" -o "$@"

myos.bin: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

initrd.img: tools/mkinitrd.py $(wildcard initrd/*)
	@mkdir -p initrd
	python3 tools/mkinitrd.py initrd initrd.img

run: myos.bin initrd.img
	qemu-system-i386 -m 4G -kernel myos.bin -initrd initrd.img -serial stdio

clean:
	rm -rf $(OBJ_DIR) myos.bin initrd.img
