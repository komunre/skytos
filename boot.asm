org 0x7C00
bits 16

%define ENDL 0x0D 0x0A

; FAT12
jmp short start
nop

bdb_oem:                db 'MSWIN4.1'
bdb_bytes_per_sector:   dw 512
bdb_sectors_per_cluster:db 1
bdb_reserved_sectors:   dw 1
bdb_fat_count:          db 2
bdb_dir_entries_count:  dw 0xE0
bdb_total_sectors:      dw 2880
bdb_media_descriptor_type:  db 0xF0
bdb_sectors_per_fat:    dw 9
bdb_sectors_per_track:  dw 18
bdb_heads:              dw 2
bdb_hidden_sectors:     dd 0
bdb_large_sector_count: dd 0

ebr_drive_number:       db 0
                        db 0
ebr_signature:          db 29h
ebr_volume_id:          db 0x12, 0x34, 0x56, 0x78,
ebr_volume_label:       db 'SKYTOS   11'
ebr_system_id:          db 'FAT12   '



start:
    jmp main

puts:
    push si
    push ax
.puts_loop:
    lodsb
    or al, al
    jz .puts_done
    mov ah, 0x0e
    int 0x10
    jmp .puts_loop
.puts_done:
    pop ax
    pop si
    ret

main:
    ; setup data segments
    mov ax, 0 ; ds/es aren't writeable directly
    mov ds, ax
    mov es, ax

    ; setup stack
    mov ss, ax
    mov sp, 0x7C00

    push es
    push word .main_after
    retf
.main_after

    ; send loading msg
    mov si, msg_loading
    call puts

    ;mov ax, 1
    ;mov cl, 1
    ;mov bx, 0x7E00
    ;call disk_read
    ;cli
    ;hlt


    ; read drive parameters from bios
    push es
    mov ah, 08h
    int 13h
    jc floppy_error
    pop es

    and cl, 0x3F
    xor ch, ch
    mov [bdb_sectors_per_track], cx
    
    inc dh
    mov [bdb_heads], dh

    ; compute LBA to root directory
    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx
    add ax, [bdb_reserved_sectors]
    push ax
    
    ; compute size of root directory
    mov ax, [bdb_dir_entries_count]
    shl ax, 5
    xor dx, dx
    div word [bdb_bytes_per_sector]
    
    test dx, dx
    jz .root_dir_after
    inc ax

.root_dir_after:
    ; read root
    mov cl, al ; cl = number of sectors to read
    pop ax
    mov dl, [ebr_drive_number]
    mov bx, buffer
    call disk_read

    mov si, msg_kernel_search
    call puts

    ; search for kernel.bin
    xor bx, bx
    mov di, buffer

.search_kernel:
    mov si, file_kernel_bin
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je .found_kernel

    add di,32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_kernel
    jmp kernel_not_found_error

.found_kernel:
    mov ax, [di + 26] ; first cluster
    mov [kernel_cluster], ax

    ; load FAT
    mov ax, [bdb_reserved_sectors]
    mov bx, buffer
    mov cl, [bdb_sectors_per_fat]
    mov dl, [ebr_drive_number]
    call disk_read
    ; read kernel
    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx
    mov bx, KERNEL_LOAD_OFFSET

.load_kernel_loop:
    
    mov ax, [kernel_cluster]
    add ax, 31 ; HARDCODED!!!!!!!!!!!
    mov cl, 1
    mov dl, [ebr_drive_number]
    call disk_read
    add bx, [bdb_bytes_per_sector]
    
    mov ax, [kernel_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz .even

.odd:
    shr ax, 4
    jmp .next_cluster_after
.even:
    and ax, 0x0FFF
.next_cluster_after:
    cmp ax, 0x0FF8
    jae .read_finish

    mov [kernel_cluster], ax
    jmp .load_kernel_loop

.read_finish:
    mov dl, [ebr_drive_number] ; pass boot device
    mov ax, KERNEL_LOAD_SEGMENT ; set segment registers 
    mov ds, ax
    mov es, ax

    ; Kernel call!
    jmp KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET

    cli
    hlt

lba_to_chs:
    push ax
    push dx

    xor dx, dx
    div word [bdb_sectors_per_track]
    
    inc dx
    mov cx, dx

    xor dx, dx
    div word [bdb_heads]

    mov dh, dl
    mov ch, al
    shl ah, 6
    or cl, ah

    pop ax
    mov dl, al
    pop ax
    ret

; Paramteters:
;   - ax: LBA
;   - cl: sectors count
;   - dl: drive number
;   -es:bx memory address pointer (where to save)
disk_read:
    push ax
    push bx
    push cx
    push dx
    push di

    push cx
    call lba_to_chs
    pop ax
    mov ah, 02h
    mov di, 5

.retry:
    pusha
    stc
    int 13h
    jnc .done
    ; failed
    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry
    jmp floppy_error

.done:
    popa
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc floppy_error
    popa
    ret

floppy_error:
    mov si, msg_read_failed
    call puts
    jmp fail

kernel_not_found_error:
    mov si, msg_kernel_not_found
    call puts
    jmp fail

fail:
    ;mov si, msg_generic_boot_error
    ;call puts
    cli
    hlt
halt:
    cli
    hlt

msg_generic_boot_error: db 'BOOT ERROR', ENDL, 0
msg_loading:            db 'Loading...', ENDL, 0
msg_read_failed:        db 'Disk read fail', ENDL, 0
msg_kernel_not_found:   db 'Stage2 not found', ENDL, 0
msg_kernel_search:      db 'Stage2 srch', ENDL, 0
file_kernel_bin:        db 'STAGE   BIN'
kernel_cluster:         dw 0

KERNEL_LOAD_SEGMENT     equ 0x2000
KERNEL_LOAD_OFFSET      equ 0

times 510-($-$$) db 0
dw 0AA55h

buffer: