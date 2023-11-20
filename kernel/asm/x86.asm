bits 16

section _TEXT class=CODE

global __U4D
__U4D:
    shl edx, 16
    mov dx, ax
    mov eax, edx
    xor edx, edx

    shl ecx, 16
    mov cx, bx

    div ecx
    mov ebx, edx
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16

    ret

global __U4M
__U4M:
    shl edx, 16
    mov dx, ax
    mov eax, edx
    
    shl ecx, 16
    mov cx, bx

    mul ecx
    mov edx, eax
    shr edx, 16

    ret


global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:
    ; new call frame
    push bp
    mov bp, sp

    push bx

    ; [bp + 0] - old call frame
    ; [bp + 2] - return address (small memory model => 2 bytes)
    ; [bp + 4] - first argument (character)
    ; [bp + 6] - second argument (page)
    mov ah, 0Eh
    mov al, [bp+4]
    mov bh, [bp+6]
    
    int 10h

    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret

global _x86_div64_32
_x86_div64_32:
    push bp
    mov bp, sp

    push bx

    mov eax, [bp + 8]
    mov ecx, [bp + 12]
    xor edx, edx
    div ecx

    mov bx, [bp + 16]
    mov [bx + 4], eax

    mov eax, [bp + 4]
    div ecx

    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    mov sp, bp
    pop bp
    ret

;bool _cdecl x86_Disk_Reset(uint8_t drive);
global _x86_Disk_Reset
_x86_Disk_Reset:
    push bp
    mov bp, sp

    mov ah, 0
    mov dl, [bp + 4]
    stc
    int 13h

    mov ax, 1
    sbb ax, 0

    mov sp, bp
    pop bp
    ret

;bool _cdecl x86_Disk_Read(uint8_t drive,
;                        uint16_t cylinder,
;                        uint16_t head,
;                        uint16_t sector,
;                        uint8_t count,
;                        uint8_t far* dataOut);
global _x86_Disk_Read
_x86_Disk_Read:
    push bp
    mov bp, sp

    push bx
    push es

    mov dl, [bp + 4] ; dl drive
    
    mov ch, [bp + 6] ; ch cylinder (lower 8 bits)
    mov cl, [bp + 7] ; cl cylinder to bits 6-7
    shl cl, 6

    mov dh, [bp + 8] ; dh head

    mov al, [bp + 10] ; al sector to bits 0-5
    and al, 3Fh
    or cl, al
    
    mov al, [bp + 12] ; al count
    
    mov bx, [bp + 16] ; es:bx far pointer
    mov es, bx
    mov bx, [bp + 14] ; :bx part

    mov ah, 02h
    stc
    int 13h

    mov ax, 1
    sbb ax, 0

    pop es
    pop bx

    mov sp, bp
    pop bp
    ret
    

;bool _cdecl x86_Disk_GetDriveParams(uint8_t drive,
;                                    uint8_t* driveTypeOut,
;                                    uint16_t* cylindersOut,
;                                    uint16_t sectorsOut,
;                                    uint16_t headsOut);
global _x86_Disk_GetDriveParams
_x86_Disk_GetDriveParams:
    push bp
    mov bp, sp

    push es
    push bx
    push si
    push di

    mov dl, [bp + 4]
    mov ah, 08h
    mov di, 0
    mov es, di
    stc
    int 13h

    mov ax, 1
    sbb ax, 0

    mov si, [bp + 6]
    mov [si], bl ; drive type out

    mov bl, ch ; max cylinder number
    mov bh, cl ; maximum sector number
    shr bh, 7
    
    mov si, [bp + 8]
    mov [si],bx ; cylinders

    xor ch, ch ; sectors
    and cl, 3Fh
    mov si, [bp + 10]
    mov [si], cx ; sectors out

    mov cl, dh ; heads
    mov si, [bp + 12]
    mov [si], cx ; heads out

    pop di
    pop si
    pop bx
    pop es

    mov sp, bp
    pop bp
    ret