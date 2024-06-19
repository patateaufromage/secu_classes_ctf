#include <stdint.h>
#include <stdio.h>

int main() 
{
	uint32_t check = 0xdeadbeef;
	char buffer[0x200];

	puts("Whats your name ? :)");
	gets(buffer);

	printf("Hello %s !\n", buffer);

	if (check == 0xcafec0de)
		puts("YOU WIN!!");
	else if (check != 0xdeadbeef)
		printf("Check overwritten with %#x bravo, almost done\n", check);
	else
		puts("NOOB");
}

