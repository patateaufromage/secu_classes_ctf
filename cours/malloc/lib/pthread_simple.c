#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_message_function( void *ptr)
{
     char *message;
     message = (char *) ptr;
	 printf("recv %p\n", ptr);
     printf("%s \n", message);
	 return ptr;
}

int main()
{
     pthread_t thread1, thread2;
     char *message1 = "Thread 1";
     char *message2 = "Thread 2";
     void *iret1, *iret2;

     pthread_create(&thread1, NULL, (void*(*)(void*)) print_message_function, (void*) message1);
     pthread_create(&thread2, NULL, (void*(*)(void*)) print_message_function, (void*) message2);

     pthread_join(thread1, &iret1);
     pthread_join(thread2, &iret2); 

     printf("Thread 1 returns: %p\n", iret1);
     printf("Thread 2 returns: %p\n", iret2);
     exit(0);
}
