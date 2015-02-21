#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <processwork.h>     //Nostro

#define SERV_PORT 5042
#define MAX_PROLE_NUM 10    //Massimo numero processi concorrenti (oltre al padre). Si suppone che ogni processo si divida in thread.

int main()
{
	int fd, sock, i;
	struct sockaddr_in servaddr;
	pid_t pid[MAX_PROLE_NUM];

	if ((fd = open("error.log", O_CREAT | O_WRONLY, 0666)) == -1) {
		perror("Error in opening error.log");
		return (EXIT_FAILURE);
	}
	if (dup2(fd, STDERR_FILENO) == -1) {
		perror("Error in opening error.log");
		return (EXIT_FAILURE);
	}

	if (close(fd)==-1){
		perror("Error in closing error.log");
		return (EXIT_FAILURE);
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Error in socket");
		return (EXIT_FAILURE);
	}

	if (memset((void*)&servaddr, 0, sizeof(servaddr))==-1)
	{
		perror("Error in memset");
		return (EXIT_FAILURE);
	}
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr(htonl(INADDR_ANY));
	servaddr.sin_port=htons(SERV_PORT);

	if(bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
	{
		perror("Error in bind");
		return (EXIT_FAILURE);
	}
	
	if (listen (sock, SOMAXCONN)<0)   //SOMAXCONN=acccetta connessioni finchè il SO non getta la spugna
	{
		perror("Error in listen");
		return (EXIT_FAILURE);
	}
	
	for (i=0; i<=MAX_PROLE_NUM; i++)
	{
		switch (pid[i]=fork())    //Funziona?
		{
			case -1:
					perror("Error in prefork");
					return (EXIT_FAILURE);
			case 0:
				Process_Work(sock);  //Da aggiungere in un nostro header
			default:
				continue;
		}
	}


	return 0;
}
