#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

int main() {
	int fd=0, sock=0;

	if (fd=open("error.log", O_CREAT| O_WRONLY, 0666)==-1)
	{	
		perror("Error in opening error.log);
		return (EXIT_FAILURE);
	}
	if (dup2(fd, STDERR_FILENO)==-1)
	{
		perror("Error in opening error.log);
		return (EXIT_FAILURE);
	}
	
	if ((sock=socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		perror("Error in socket");
		return (EXIT_FAILURE);
	}
	return 0;
}
