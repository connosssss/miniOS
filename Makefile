AS = nasm
CXX = g++
LD = ld

INCLUDE_DIRS = -Isrc -Isrc/boot -Isrc/gdt -Isrc/idt -Isrc/pic -Isrc/drivers -Isrc/memory -Isrc/utils -Isrc/kernel -Isrc/memory/paging

ASFLAGS = -f elf32
CXXFLAGS = -m32 -std=c++17 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -nostdlib -Wall -Wextra $(INCLUDE_DIRS)
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

OBJ_DIR := obj

CPP_SOURCES := $(shell find src -type f -name '*.cpp')
ASM_SOURCES := $(shell find src -type f -name '*.asm')
OBJECTS     := $(CPP_SOURCES:src/%.cpp=$(OBJ_DIR)/%.o) $(ASM_SOURCES:src/%.asm=$(OBJ_DIR)/%.o)

.PHONY: all run clean

all: myos.bin

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p "$(dir $@)"
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/%.o: src/%.asm
	@mkdir -p "$(dir $@)"
	$(AS) $(ASFLAGS) "$<" -o "$@"

myos.bin: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

run: myos.bin
	qemu-system-i386 -m 4G -kernel myos.bin -serial stdio

clean:
	rm -rf $(OBJ_DIR) myos.bin
