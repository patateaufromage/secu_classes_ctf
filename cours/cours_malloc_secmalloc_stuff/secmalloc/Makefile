CC = gcc -g
CFLAGS = -Wall -g -Werror -Wextra -std=gnu17 -I ./include
OBJ = my_alloc.o

all: ${OBJ}

my_alloc.o: my_alloc.c
	$(CC) $(CFLAGS) -c my_alloc.c -o my_alloc.o

test: ${OBJ} test.o 
	$(CC) -L./lib -lcriterion -I ./include -o test test.o my_alloc.o
	./test

clean:
	$(RM) *.o *.swp .*.swo test

distclean: clean
	$(RM) test

.PHONY: all clean distclean test
