; Listing 139

global nop_aligned_64
global nop_aligned_1
global nop_aligned_15
global nop_aligned_31
global nop_aligned_63

section .text

nop_aligned_64:
    xor rax, rax
align 64
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret

nop_aligned_1:
    xor rax, rax
align 64
nop
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret

nop_aligned_15:
    xor rax, rax
align 64
%rep 15
    nop
%endrep
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret

nop_aligned_31:
    xor rax, rax
align 64
%rep 31
    nop
%endrep
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret

nop_aligned_63:
    xor rax, rax
align 64
%rep 63
    nop
%endrep
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret
