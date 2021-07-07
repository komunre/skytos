boot_src = boot.asm
boot_obj = boot.obj

kernel_src = kernel/src/string.c
kernel_obj = $(patsubst kernel/src/%.c, %.o, $(kernel_src))

FLAGS = -m32 -fno-pie -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -ffreestanding -Ikernel

all: run

$(boot_obj): $(boot_src)
	nasm -felf32 $< -o $@

kernel.o: kernel/kernel.c
	gcc -m32 -fno-pie -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@

screen.o: kernel/src/screen.c
	gcc $(FLAGS) -c $< -o $@

ps2.o: kernel/src/ps2.c
	gcc $(FLAGS) -c $< -o $@
	
$(kernel_obj): $(kernel_src)
	gcc $(FLAGS) -c $< -o $@

#otal.o: $(kernel_src)
#	gcc -m32 -fno-pie -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -ffreestanding -o total.o $^

link: $(boot_obj) kernel.o $(kernel_obj) screen.o ps2.o
	@echo kernel sources: $(kernel_src)
	@echo kernel objects: $(kernel_obj)
	ld -zmuldefs -melf_i386 -T linker.ld -o skytos.bin $^

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