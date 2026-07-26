AS = nasm
CXX = g++
LD = ld

ASFLAGS = -f elf32
CXXFLAGS = -m32 -std=c++17 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -nostdlib -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

CPP_SOURCES := $(wildcard src/*.cpp)
ASM_SOURCES := $(wildcard src/*.asm)
OBJECTS     := $(CPP_SOURCES:src/%.cpp=src/%.o) $(ASM_SOURCES:src/%.asm=src/%.o)

.PHONY: all run clean

all: myos.bin

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/%.o: src/%.asm
	$(AS) $(ASFLAGS) $< -o $@

myos.bin: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

run: myos.bin
	qemu-system-i386 -kernel myos.bin -serial stdio

clean:
	rm -f src/*.o *.o myos.bin