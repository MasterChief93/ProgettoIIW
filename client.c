#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define SERV_PORT 5042
#define MAXLINE 1024

int main()
{
	int sock;
	struct sockaddr_in servaddr;
	
	errno=0;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Error in socket");
		return (EXIT_FAILURE);
	}

	if (memset((void*)&servaddr, 0, sizeof(servaddr))==NULL)
	{
		perror("Error in memset");
		return (EXIT_FAILURE);
	}
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(SERV_PORT);
	if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <=0) {
		perror("errore in inet_pton");
		exit(-1);
	}
	
	if (connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)))
	{
		perror("Error in connect");
		return (EXIT_FAILURE);
	}
	
	
	return EXIT_SUCCESS;
}	

