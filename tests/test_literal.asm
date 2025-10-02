    .section .data
    .section .text
    .global _main
_main:
    push    rbp
    mov     rbp, rsp
    mov     rax, 42
    mov     rsp, rbp
    pop     rbp
    ret
