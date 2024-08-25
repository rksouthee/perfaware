global process_bytes

section .text

; rcx = number of bytes to process
; rdx = pointer to the bytes
; r8 = mask
process_bytes:
    xor r9, r9
    mov rax, rdx
    align 64
.loop:
    vmovdqu ymm0, [rax]
    vmovdqu ymm0, [rax + 0x20]
    vmovdqu ymm0, [rax + 0x40]
    vmovdqu ymm0, [rax + 0x60]
    vmovdqu ymm0, [rax + 0x80]
    vmovdqu ymm0, [rax + 0xa0]
    vmovdqu ymm0, [rax + 0xc0]
    vmovdqu ymm0, [rax + 0xe0]

    add r9, 0x100
    and r9, r8

    mov rax, rdx
    add rax, r9

    sub rcx, 0x100
    jnz .loop

    ret