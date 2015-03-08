#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sqlite3.h>
#include "threadwork.h"    //Nostro
#include "processwork.h"
#include "db.h"

#define BUFF_SIZE 1024

int Thread_Work(int connsd, int fdl, sqlite3 *db)
{
	ssize_t readn, writen;
	
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
			else if (errno == EINTR) 
				readn = 0;
			else {
				perror("read");
				exit(EXIT_FAILURE);
			}
		}
		else if (readn == 0) break;
		if (buff[strlen(buff)-1] == '\0') break;
		nleft -= readn;
		ptr += readn;
	}
	if (buff[strlen(buff)-1] != '\0') {
		buff[strlen(buff)-1] = 0;
	}
	if (strlen(buff) < 1) {
		perror("No string");
		exit(EXIT_FAILURE);

	}
	printf("Ho finito il while\n");
	fflush(stdout);
	char *saveptr;
	ptr = strtok_r(buff,"\r\n",&saveptr);
	ptr = strtok_r(ptr," ",&saveptr); //ptr conterrà il metodo
	printf("Ho fatto le strtok %s\n",ptr);
	fflush(stdout);
	if (strcmp(ptr,"GET") == 0) {
		printf("Bella zio\n");
		fflush(stdout);

		//chiama funzione GET
	}
	else if (strcmp(ptr,"HEAD") == 0) {
		printf("Ho fatto le HEAD\n");
		fflush(stdout);
		//chiama funzione HEAD
		}
	else {
		printf("Scrivo\n");
		fflush(stdout);
		writen = send(connsd,"HTTP/1.1 405 Method Not Allowed\r\n\r\n",strlen("HTTP/1.1 405 Method Not Allowed\r\n\r\n"),MSG_DONTWAIT);

		if (writen == 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
		printf("Ho scritto\n");
		fflush(stdout);
	}
	printf("Chiudo\n");
	fflush(stdout);
	if (shutdown(connsd,SHUT_RDWR) < 0) {
		perror("shutdown");
		exit(EXIT_FAILURE);
	}
	printf("Shutdown fatto\n");
	fflush(stdout);
	if (close(connsd) < 0) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	printf("Tutto fatto\n");
	fflush(stdout);
	return EXIT_SUCCESS;
}
