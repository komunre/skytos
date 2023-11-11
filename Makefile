boot_src = boot.asm
boot_obj = boot.obj

CC16=wcc
LD16=wlink

CFLAGS16=-4 -d3 -s -wx -ms -zl -zq -za99 -Iinclude/

kernel_src:=kernel/stdio.c kernel/disk.c kernel/fat.c kernel/string.c
kernel_obj:=$(patsubst kernel/%.c, %.obj, $(kernel_src))
kernel_asm_src:=kernel/asm/x86.asm
kernel_asm_bin:=$(patsubst kernel/asm/%.asm, %.bin, $(kernel_asm_src))

FLAGS = -m32 -fno-pie -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -ffreestanding -Iinclude/

.PHONY: all run link iso clean floppy_image 

all: floprun

$(boot_obj): $(boot_src)
	nasm -fbin $< -o $@

$(kernel_asm_bin): $(kernel_asm_src)
	nasm -fobj $< -o $@
stage2_asm.bin: stage2/boot_stage2.asm
	nasm -fobj $< -o $@
stage2_c.obj: stage2/boot_stage2.c
	$(CC16) $(CFLAGS16) -fo=$@ $<
stage2.bin: stage2_asm.bin $(kernel_asm_bin) stage2_c.obj FUCK_YOU
	$(LD16) NAME stage2.bin FILE \{ stage2_asm.bin $(kernel_asm_bin) stage2_c.obj $(kernel_obj) \} OPTION MAP=stage2.map @linker.lnk


tools_fat12: tools/fat/fat.c
	gcc -o $@ tools/fat/fat.c

	
#$(kernel_obj): $(kernel_src)
#	$(CC16) $(CFLAGS16) -fo=$@ $<

FUCK_YOU:
	$(CC16) $(CFLAGS16) -fo=stdio.obj kernel/stdio.c
	$(CC16) $(CFLAGS16) -fo=disk.obj kernel/disk.c
	$(CC16) $(CFLAGS16) -fo=fat.obj kernel/fat.c
	$(CC16) $(CFLAGS16) -fo=string.obj kernel/string.c



#kernel.o: kernel/kernel.c
#	gcc $(FLAGS) -c $< -o $@

#screen.o: kernel/src/screen.c
#	gcc $(FLAGS) -c $< -o $@

#port.o: kernel/src/port/port.c
#	gcc $(FLAGS) -c $< -o $@

#ps2.o: kernel/src/ps2.c
#	gcc $(FLAGS) -c $< -o $@

#otal.o: $(kernel_src)
#	gcc -m32 -fno-pie -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -ffreestanding -o total.o $^

#link_old: $(boot_obj) kernel.o $(kernel_obj) screen.o ps2.o port.o
#	@echo kernel sources: $(kernel_src)
#	@echo kernel objects: $(kernel_obj)
#	ld -zmuldefs -melf_i386 -T linker.ld -o skytos.bin $^

link: $(boot_obj) stage2.bin $(kernel_obj)
	@echo kernel sources: $(kernel_src)
	@echo kernel objects: $(kernel_obj)
#	ld -zmuldefs -melf_i386 -T linker.ld -o skytos.bin kernel.o $(kernel_obj) screen.o ps2.o port.o
	nasm -fbin unused/test_kernel.asm -o skytos.bin
	cp $(boot_obj) boot.bin  
#	cp file3.txt skytos.bin

iso: link
	mkdir -p iso/boot/grub
	cp skytos.bin iso/boot/skytos.bin
	grub-mkrescue -o skytos.iso iso/

main_floppy.img: link
	dd if=/dev/zero of=main_floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "SKOS" main_floppy.img
	dd if=boot.bin of=main_floppy.img conv=notrunc
	mcopy -i main_floppy.img stage2.bin "::stage.bin"
	mcopy -i main_floppy.img skytos.bin "::skytos.bin"
	mcopy -i main_floppy.img test.txt "::test.txt"

floppy_image: link main_floppy.img
	

floprun: floppy_image
	qemu-system-i386 -m 2048 -fda main_floppy.img

run: iso
	qemu-system-i386 -m 2048 skytos.iso

clean:
	rm -f *.o
	rm -f *.obj
	rm -f boot.obj
	rm -f *.bin
	rm -f main_floppy.img
	rm -f iso/boot/skytos.bin
	rm -f *.iso