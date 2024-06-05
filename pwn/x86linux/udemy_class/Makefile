CC=/usr/bin/gcc -m32

all:
	$(CC) -fno-stack-protector -z execstack -D_FORTIFY_SOURCE=0 -no-pie vulnerable.c -o vulnerable

clean:
	rm -f vulnerable

make re:
	rm -f vulnerable && make all
