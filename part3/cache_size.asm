global process_bytes

section .text

; rcx = number of bytes to process
; rdx = pointer to the bytes
; r8 = mask
process_bytes:
    xor rax, rax
    align 64
.loop:
    mov r10, rdx
    add r10, rax
    mov r9, [r10]
    mov r9, [r10 + 8]
    mov r9, [r10 + 16]
    mov r9, [r10 + 24]
    add rax, 32
    and rax, r8
    sub rcx, 32
    jnz .loop
    ret