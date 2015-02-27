#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "threadwork.h"    //Nostro
#include "processwork.h"

#define BUFF_SIZE 1024

int Thread_Work(int connsd)
{
	ssize_t readn,writen;
	
	size_t nleft;
	char *buff;
	buff = malloc(sizeof(char) * BUFF_SIZE);
	if (buff == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	char *ptr = buff;
	/*if (fcntl(connsd, F_SETFL, O_NONBLOCK) == -1) {  		// set to non-blocking
		perror("fcntl");
		exit(EXIT_FAILURE);
	}*/
	errno = 0;
	nleft = BUFF_SIZE;
	
	while(nleft > 0) {
		if ((readn = recv(connsd, ptr, nleft, MSG_DONTWAIT)) < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				*ptr = '\0';
				break;
			}
			else {
				perror("read");
				exit(EXIT_FAILURE);
			}
		}
		else if (readn == 0) break;
		nleft -= readn;
		ptr += readn;
	}
	writen = write(STDOUT_FILENO,buff,strlen(buff));
	if (writen == 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}
	if (close(connsd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
