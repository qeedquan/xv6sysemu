global retuser
global jumpureg

%define SYSSTACK 0x4000000

section .text

retuser:
	mov esp, [esp + 4]
	jmp [esp]

jumpureg:
	mov  eax, [esp + 4]
	push dword [eax + 4*16]
	popf
	mov  edi, [eax + 4*4]
	mov  esi, [eax + 4*5]
	mov  ebp, [eax + 4*6]
	mov  ebx, [eax + 4*8]
	mov  edx, [eax + 4*9]
	mov  ecx, [eax + 4*10]
	mov  esp, [eax + 4*7]
	mov  eax, [eax + 4*11]
	jmp  [SYSSTACK]

section .note.GNU-stack
