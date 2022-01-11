.586p

descr struc
    lim dw 0
    base_1 dw 0
    base_m db 0
    attr_1 db 0
    attr_2 db 0
    base_h db 0
descr ends

data segment use16
    gdt_null descr <0, 0, 0, 0, 0, 0>
    gdt_data descr <data_size - 1, 0, 0, 92h, 0, 0>
    gdt_code descr <code_size - 1, 0, 0, 98h, 0, 0>
    gdt_stack descr <255, 0, 0, 98h, 0, 0>
    gdt_screen descr <3999, 8000h, 0BH, 92h, 0, 0>

    gdt_size = $-gdt_null

    pdescr df 0
    sym db 1
    attr db 1EH

    msg db 27, ' [31; 42m return to real mode], 27, ' [0m$]'
    data_size = $-gdt_null
data ends

text segment use16
    assume CS:text, DS:data;

main proc
start:
    xor eax, eax
    mov ax, data
    mov ds, ax
    shl eax, 4
    mov ebp, eax
    mov bx, offset gdt_data
    mov bx.base_1, ax
    shr eax, 16
    mov bx.base_m, al
    ;
    xor eax, eax
    mov ax, CS
    shl eax, 4
    mov bx, offset gdt_code
    mov bx.base_1, ax
    shr eax, 16
    mov bx.base_m, al
    ;
    xor eax, eax
    mov ax, ss
    shl eax, 4
    mov bx, offset gdt_stack
    mov bx.base_1, ax;
    shr eax, 16
    mov bx.base_m, al
    ;
    mov dword ptr pdescr + 2, ebp
    mov word ptr pdescr, gdt_size - 1
    lgdt pdescr
    ;
    mov ax, 40h
    mov es, ax
    mov word ptr es:[67h], offset return;
    mov es:[69h], CS
    mov al, 0fh
    out 70h, al
    mov al, 0ah
    out 71h, al
    cli
    ;
    mov eax, cr0;
    or eax, 1;
    mov cr0, eax
    ;
    db 0eah
    dw offset continue
    dw 16
continue:
    mov ax, 8
    mov ds, ax
    ;
    mov ax, 24
    mov ss, ax;
    ;
    mov ax, 32;
    mov es, ax;
    ;
    mov di, 1920;
    mov cx, 80
    mov ax, word ptr sym
scrn:
    stosw
    inc al
    loop scrn
    ;
    mov al, 0feh
    out 64h, al;
    hlt
return:
    mov ax, data
    mov ds, ax;
    mov ax, stk
    mov ss, ax
    mov sp, 356
    sti
    ;
    mov ah, 09h
    mov dx, offset msg
    int 21h
    mov ax, 4c00h
    int 21h
    
    code_size = $-start
main endp

text ends

stk segment stack use16
    db 256 dup('^');
stk ends;
    end main