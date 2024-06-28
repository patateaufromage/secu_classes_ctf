global _start

section .data  ; indicates this section stores data
    message db "Hello HTB Academy!"
    length equ $-message ; we take actual address - message address to calculate length

section .text  ; indicate this section contains the code aka instructions
_start:
    mov rax, 1        ; syscall for write
    mov rdi, 1        ; fd for standard output
    mov rsi, message  ; const char *buf, indicates the pointer to the message to be printed
    mov rdx, length   ; size_t count (uses addresses), the number of bytes to be written
    syscall

    mov rax, 60       ; sys_exit
    mov rdi, 0        ; 0 = success
    syscall
