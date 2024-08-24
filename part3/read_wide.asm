;  ========================================================================
;  LISTING 150
;  ========================================================================

global read_4x3
global read_8x3
global read_16x3
global read_32x3

section .text

read_4x3:
    xor rax, rax
	align 64
.loop:
    mov r8d, [rdx]
    mov r8d, [rdx + 4]
    mov r8d, [rdx + 8]
    add rax, 12
    cmp rax, rcx
    jb .loop
    ret

read_8x3:
    xor rax, rax
	align 64
.loop:
    mov r8, [rdx]
    mov r8, [rdx + 8]
    mov r8, [rdx + 16]
    add rax, 24
    cmp rax, rcx
    jb .loop
    ret

read_16x3:
    xor rax, rax
    align 64
.loop:
    vmovdqu xmm0, [rdx]
    vmovdqu xmm0, [rdx + 16]
    vmovdqu xmm0, [rdx + 32]
    add rax, 48
    cmp rax, rcx
    jb .loop
    ret

read_32x3:
    xor rax, rax
	align 64
.loop:
    vmovdqu ymm0, [rdx]
    vmovdqu ymm0, [rdx + 32]
    vmovdqu ymm0, [rdx + 64]
    add rax, 96
    cmp rax, rcx
    jb .loop
    ret
