#include <sys/mman.h>
#include <stdio.h>

int main()
{
    // simple allocation
    char *ptr = mmap(NULL, 2600, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    for (int i = 0; i < 2600; i += 1)
        ptr[i] = 'X';
    ptr[2600] = 0;
    printf("Chunk %s\n", ptr);
    munmap(ptr, 2600);
}
