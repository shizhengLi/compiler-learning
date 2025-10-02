    .section .data
    .section .text
    .global _main
_main:
    push    rbp
    mov     rbp, rsp
    mov     rax, 5
    push    rax
    mov     rax, 3
    pop     rbx
    add     rax, rbx
    mov     rsp, rbp
    pop     rbp
    ret
