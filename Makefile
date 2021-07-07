boot_src = boot.asm
boot_obj = boot.obj

all: run

$(boot_obj): $(boot_src)
	nasm -felf32 $< -o $@

kernel.o: kernel/kernel.c
	gcc -m32 -fno-pie -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@

link: $(boot_obj) kernel.o
	ld -melf_i386 -T linker.ld -o skytos.bin $^

iso: link
	mkdir -p iso/boot/grub
	cp skytos.bin iso/boot/skytos.bin
	grub-mkrescue -o skytos.iso iso/
	
run: iso
	qemu-system-i386 -m 2048 skytos.iso

clean:
	rm *.o
	rm *.bin
	rm *.iso
	rm iso/boot/skytos.bin