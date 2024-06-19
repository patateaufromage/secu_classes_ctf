#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define _SCHED_H 1
#define __USE_GNU 1
#include <bits/sched.h>

#define STACK_SIZE 4096

int func(void *arg) {
    (void) arg;
    printf("Inside func.\n");
    sleep(1);
    printf("Terminating func...\n");

    return 0;
}

int main() {
    printf("This process pid: %u\n", getpid());
    void *child_stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANON | MAP_STACK, -1, 0);
    int thread_pid;

    printf("Creating new thread...\n");
    thread_pid = clone(&func, (void*)((long)child_stack + STACK_SIZE),
            CLONE_SIGHAND|CLONE_FS|CLONE_VM|CLONE_FILES, NULL);
    printf("Done! Thread pid: %d\n", thread_pid);

    sleep(2);

    return 0;
}
