org 0x0
bits 16

%define ENDL 0x0D, 0x0A

start:
    mov si, msg_hello
    call puts
.halt:
    cli
    hlt

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

msg_hello:      db 'Hello from kernel', ENDL, 0