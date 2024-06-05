#include<stdio.h>
#include<string.h>

void vuln_func(char *input);

int main(int argc, char *argv[])
{
	if(argc>1)
	vuln_func(argv[1]);
}

void vuln_func(char *input)
{
	char buffer[256];
	strcpy(buffer, input);
}

