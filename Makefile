AS = nasm
CXX = g++
LD = ld

ASFLAGS = -f elf32
CXXFLAGS = -m32 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -nostdlib -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

OBJS = src/boot.o src/kernel.o src/gdt.o src/gdtasm.o

all: myos.bin

src/boot.o: src/boot.asm
	$(AS) $(ASFLAGS) $< -o $@

src/kernel.o: src/kernel.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/gdt.o: src/gdt.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/gdtasm.o: src/gdt_asm.asm
	$(AS) $(ASFLAGS) $< -o $@

myos.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

run: myos.bin
	qemu-system-i386 -kernel myos.bin -serial stdio

clean:
	rm -f src/*.o *.o myos.bin