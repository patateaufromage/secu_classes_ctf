#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>


int main(void)
{
    int fd = open("test2.txt", O_CREAT | O_RDWR | O_SYNC, S_IRUSR | S_IWUSR);
    pid_t pid = fork();
    if (pid == 0)
    {
        // child process
        printf("printf from child.\n");
        char buf[] = "toto\n";
        write(fd, buf, 5);
        close(fd);
        for (int i = 0; i < 4; i += 1)
            sleep(1);
    }
    else
    {
        // parent process
        printf("child PID is %d\n", pid);
        //int status = 0;
        sleep(1);
        //waitpid(pid, &status, 0);
        char buf[20];
        lseek(fd, 0, SEEK_SET);
        read(fd, buf, sizeof(buf));
        printf("CONTENT : %s\n", buf);
        close(fd);
        while (2600)
            sleep(1);
    }
}
