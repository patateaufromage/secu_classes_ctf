#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>

int main()
{
    char *ptr = mmap(NULL, 2600, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    pid_t pid = fork();
    if (pid == 0)
    {
        // child process write on SHARED MEMORY
        for (int i = 0; i < 2600; i += 1)
            ptr[i] = 'X';
        ptr[2600] = 0;
    }
    else
    {
        int status = 0;
        // wait end of child process
        waitpid(pid, &status, 0);
        printf("Chunk %s\n", ptr);
        munmap(ptr, 2600);
    }
}
