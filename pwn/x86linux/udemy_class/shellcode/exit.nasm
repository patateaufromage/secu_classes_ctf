global _start

_start:
	xor eax, eax
	mov al, 1
	xor ebx, ebx
	mov bl, 2 ; will call exit with argument 2
	int 0x80   ; used to invoke the syscall with the value contained in eax
