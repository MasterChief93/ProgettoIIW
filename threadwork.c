#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include <errno.h>
#include "threadwork.h"    //Nostro
#include "processwork.h"

#define BUFF_SIZE 1024

int Thread_Work(int connsd)
{
	(void) connsd;
	ssize_t readn,writen,totread;
	char *buff;
	buff = malloc(sizeof(char) * BUFF_SIZE);
	if (buff == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	errno = 0;
	totread = BUFF_SIZE;
	while(totread > 0) {
		readn = read(connsd, buff, sizeof(buff));
		if (readn < 0) {
			if (errno == EINTR)
				readn = 0;
			else {
				perror("write");
				exit(EXIT_FAILURE);
			}
		}
		else if (readn == 0) break;
		totread -= readn;
		buff += readn;
	}
	printf("%s",buff);
	fflush(stdout);
	writen = write(STDOUT_FILENO,buff,sizeof(buff));
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
