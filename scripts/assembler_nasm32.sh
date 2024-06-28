#!/bin/bash

fileName="${1%%.*}"

# compile
nasm -f elf32 ${fileName}".nasm" -o ${fileName}".o"

# link
ld -m elf_i386 ${fileName}".o" -o ${fileName}

# execute the program or run gdb if debug symbol has been entered:
[ "$2" == "-g" ] && gdb -q ${fileName} || ./${fileName}


# exemple pour assembler linker et run via gdb: ./assembler32.sh helloWorld.s -g
