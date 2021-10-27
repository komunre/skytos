CC = i386-elf-gcc
LD = ld

boot_src = boot.asm
boot_obj = boot.obj

kernel_src = $(shell find kernel/ -name "*.c")
kernel_obj = $(patsubst %.c, %.o, $(kernel_src))

FLAGS = -fno-pie -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -ffreestanding

all: run

$(boot_obj): $(boot_src)
	nasm -felf32 $< -o $@


%.o: %.c $(kernel_src)
	$(CC) $(FLAGS) -c $< -o $@

#otal.o: $(kernel_src)
#	gcc -m32 -fno-pie -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -ffreestanding -o total.o $^

link: $(boot_obj) $(kernel_obj)
	@echo kernel sources: $(kernel_src)
	@echo kernel objects: $(kernel_obj)
	$(CC) -T linker.ld -o skytos.bin $(FLAGS) $^

iso: link
	mkdir -p iso/boot/grub
	cp skytos.bin iso/boot/skytos.bin
	grub-mkrescue -o skytos.iso iso/
	
drive: 
	qemu-img create -f raw test.img 512M

run: iso drive
	qemu-system-i386 -m 2048 skytos.iso -drive format=raw,file=test.img

clean:
	rm $(kernel_obj)
	rm *.bin
	rm *.iso
	rm iso/boot/skytos.bin