global retuser
global jumpureg

%define SYSSTACK 0x4000000

section .text

retuser:
	mov rsp, rdi
	mov rdi, rsi
	mov rsi, rdx
	jmp [rsp]

jumpureg:
	push qword [rdi + 8*17]
	popf
	mov  r8, [rdi + 8*0]
	mov  r9, [rdi + 8*1]
	mov  r10, [rdi + 8*2]
	mov  r11, [rdi + 8*3]
	mov  r12, [rdi + 8*4]
	mov  r13, [rdi + 8*5]
	mov  r14, [rdi + 8*6]
	mov  r15, [rdi + 8*7]
	mov  rsi, [rdi + 8*9]
	mov  rbp, [rdi + 8*10]
	mov  rbx, [rdi + 8*11]
	mov  rdx, [rdi + 8*12]
	mov  rax, [rdi + 8*13]
	mov  rcx, [rdi + 8*14]
	mov  rsp, [rdi + 8*15]
	mov  rdi, [rdi + 8*8]
	jmp  [SYSSTACK]

section .note.GNU-stack
