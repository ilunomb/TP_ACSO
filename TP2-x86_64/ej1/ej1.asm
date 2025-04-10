; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat
extern strlen
extern strcat


string_proc_list_create_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        mov     edi, 16
        call    malloc
        mov     QWORD [rbp-8], rax
        cmp     QWORD [rbp-8], 0
        jne     .L2
        mov     eax, 0
        jmp     .L3
.L2:
        mov     rax, QWORD [rbp-8]
        mov     QWORD [rax], 0
        mov     rax, QWORD [rbp-8]
        mov     QWORD [rax+8], 0
        mov     rax, QWORD [rbp-8]
.L3:
        leave
        ret




string_proc_node_create_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 32
        mov     eax, edi
        mov     QWORD [rbp-32], rsi
        mov     BYTE [rbp-20], al
        mov     edi, 32
        call    malloc
        mov     QWORD [rbp-8], rax
        cmp     QWORD [rbp-8], 0
        jne     .L5
        mov     eax, 0
        jmp     .L6
.L5:
        mov     rax, QWORD [rbp-8]
        movzx   edx, BYTE [rbp-20]
        mov     BYTE [rax+16], dl
        mov     rax, QWORD [rbp-8]
        mov     rdx, QWORD [rbp-32]
        mov     QWORD [rax+24], rdx
        mov     rax, QWORD [rbp-8]
        mov     QWORD [rax], 0
        mov     rax, QWORD [rbp-8]
        mov     QWORD [rax+8], 0
        mov     rax, QWORD [rbp-8]
.L6:
        leave
        ret

string_proc_list_add_node_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 48
        mov     QWORD [rbp-24], rdi
        mov     eax, esi
        mov     QWORD [rbp-40], rdx
        mov     BYTE [rbp-28], al
        cmp     QWORD [rbp-24], 0
        je      .L12
        movzx   eax, BYTE [rbp-28]
        mov     rdx, QWORD [rbp-40]
        mov     rsi, rdx
        mov     edi, eax
        call    string_proc_node_create_asm
        mov     QWORD [rbp-8], rax
        cmp     QWORD [rbp-8], 0
        je      .L13
        mov     rax, QWORD [rbp-24]
        mov     rax, QWORD [rax]
        test    rax, rax
        jne     .L11
        mov     rax, QWORD [rbp-24]
        mov     rdx, QWORD [rbp-8]
        mov     QWORD [rax], rdx
        mov     rax, QWORD [rbp-24]
        mov     rdx, QWORD [rbp-8]
        mov     QWORD [rax+8], rdx
        jmp     .L7
.L11:
        mov     rax, QWORD [rbp-24]
        mov     rdx, QWORD [rax+8]
        mov     rax, QWORD [rbp-8]
        mov     QWORD [rax+8], rdx
        mov     rax, QWORD [rbp-24]
        mov     rax, QWORD [rax+8]
        mov     rdx, QWORD [rbp-8]
        mov     QWORD [rax], rdx
        mov     rax, QWORD [rbp-24]
        mov     rdx, QWORD [rbp-8]
        mov     QWORD [rax+8], rdx
        jmp     .L7
.L12:
        nop
        jmp     .L7
.L13:
        nop
.L7:
        leave
        ret

string_proc_list_concat_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 64
        mov     QWORD [rbp-40], rdi
        mov     eax, esi
        mov     QWORD [rbp-56], rdx
        mov     BYTE [rbp-44], al
        cmp     QWORD [rbp-40], 0
        je      .L15
        cmp     QWORD [rbp-56], 0
        jne     .L16
.L15:
        mov     eax, 0
        jmp     .L17
.L16:
        mov     rax, QWORD [rbp-56]
        mov     rdi, rax
        call    strlen
        mov     QWORD [rbp-8], rax
        mov     rax, QWORD [rbp-40]
        mov     rax, QWORD [rax]
        mov     QWORD [rbp-16], rax
        jmp     .L18
.L20:
        mov     rax, QWORD [rbp-16]
        movzx   eax, BYTE [rax+16]
        cmp     BYTE [rbp-44], al
        jne     .L19
        mov     rax, QWORD [rbp-16]
        mov     rax, QWORD [rax+24]
        test    rax, rax
        je      .L19
        mov     rax, QWORD [rbp-16]
        mov     rax, QWORD [rax+24]
        mov     rdi, rax
        call    strlen
        add     QWORD [rbp-8], rax
.L19:
        mov     rax, QWORD [rbp-16]
        mov     rax, QWORD [rax]
        mov     QWORD [rbp-16], rax
.L18:
        cmp     QWORD [rbp-16], 0
        jne     .L20
        mov     rax, QWORD [rbp-8]
        add     rax, 1
        mov     rdi, rax
        call    malloc
        mov     QWORD [rbp-24], rax
        cmp     QWORD [rbp-24], 0
        jne     .L21
        mov     eax, 0
        jmp     .L17
.L21:
        mov     rax, QWORD [rbp-24]
        mov     BYTE [rax], 0
        mov     rdx, QWORD [rbp-56]
        mov     rax, QWORD [rbp-24]
        mov     rsi, rdx
        mov     rdi, rax
        call    strcat
        mov     rax, QWORD [rbp-40]
        mov     rax, QWORD [rax]
        mov     QWORD [rbp-16], rax
        jmp     .L22
.L24:
        mov     rax, QWORD [rbp-16]
        movzx   eax, BYTE [rax+16]
        cmp     BYTE [rbp-44], al
        jne     .L23
        mov     rax, QWORD [rbp-16]
        mov     rax, QWORD [rax+24]
        test    rax, rax
        je      .L23
        mov     rax, QWORD [rbp-16]
        mov     rdx, QWORD [rax+24]
        mov     rax, QWORD [rbp-24]
        mov     rsi, rdx
        mov     rdi, rax
        call    strcat
.L23:
        mov     rax, QWORD [rbp-16]
        mov     rax, QWORD [rax]
        mov     QWORD [rbp-16], rax
.L22:
        cmp     QWORD [rbp-16], 0
        jne     .L24
        mov     rax, QWORD [rbp-24]
.L17:
        leave
        ret