#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


int main()
{
    int fd = open("test.txt", O_CREAT | O_RDWR, 0666);
    char buf[4096] = {0};
    write(fd, buf, sizeof(buf));
    char *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == (char*)-1)
    {
        perror("Failed map\n");
        exit(1);
    }
    for (int i = 0; i < 2600; i += 1)
        ptr[i] = 'X';
    ptr[2600] = 0;
    printf("Chunk %s\n", ptr);
    msync(ptr, 4096, MS_SYNC);
    munmap(ptr, 2600);
}
