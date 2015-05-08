#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sqlite3.h>
#include "threadwork.h"    //Nostro
#include "processwork.h"
#include "db.h"

#define BUFF_SIZE 1024

int Thread_Work(int connsd, int fdl, sqlite3 *db)
{
	printf("Sono entrato nel Thread Work! Sono il tid %lld figlio di %lld\n",(long long int) pthread_self(), (long long int) getpid());
	fflush(stdout);
	ssize_t readn, writen;
	
	size_t nleft;
	char *buff;
	buff = malloc(sizeof(char) * BUFF_SIZE);
	if (buff == NULL) {
		perror("malloc");
		return EXIT_FAILURE;
	}
	char *ptr = buff;
	/*if (fcntl(connsd, F_SETFL, O_NONBLOCK) == -1) {  		// set to non-blocking
		perror("fcntl");
		exit(EXIT_FAILURE);
	}*/
	errno = 0;
	nleft = BUFF_SIZE;
	printf("Sto entrando nel while!\n");
	fflush(stdout);
	while(nleft > 0) {
		if ((readn = recv(connsd, ptr, nleft, MSG_DONTWAIT)) < 0) {
		  //if((readn = read(connsd,ptr,nleft)) < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				if (strlen(buff) == 0) {
					printf("Ti do una seconda chance\n");
					fflush(stdout);
					continue;
				} else {
					*ptr = '\0';
					printf("Il buffer contiene(0): (%s)\n",buff);
					fflush(stdout);
					break;
				}
			}
			else if (errno == EINTR) 
				readn = 0;
			else {
				perror("read");
				free(buff);
				return EXIT_FAILURE;
			}
		}
		else if (readn == 0) {
			printf("Non ho letto nulla oppure ho gia finito\n");
			fflush(stdout);
			break;
		}		
		printf("La lunghezza di buff e: %d\n",strlen(buff));
		if (strlen(buff) >= 1) {
			if (buff[strlen(buff)-1] == '\0') {
				printf("Letto tutto!\n");
				fflush(stdout);
				break;
			}
		}
		
		nleft -= readn;
		ptr += readn;
	}
	printf("Ho finito il while!\n");
	fflush(stdout);
	//printf("%s\n",buff);
	//fflush(stdout);
	printf("La lunghezza di buff e: %d\n",strlen(buff));
	fflush(stdout);
	if (buff[strlen(buff)-1] != '\0') {
		printf("Non c'era EOF e l'ho aggiunto: \"%s\"\n",buff);
		fflush(stdout);
		buff[strlen(buff)-1] = '\0';
	}
	errno = 0;
	if (strlen(buff) < 1) {
		perror("No string");
		if (shutdown(connsd,SHUT_RDWR) < 0) {
			perror("shutdown");
			free(buff);
			return EXIT_FAILURE;
		}
		printf("Shutdown fatto perche la stringa e vuota\n \"%s\"",buff);
		fflush(stdout);
		if (close(connsd) < 0) {
			perror("close");
			free(buff);
			return EXIT_FAILURE;
		}
		free(buff);
		return EXIT_FAILURE;
	}
	printf("Nessun errore fin qui\n");
	fflush(stdout);
	char *saveptr;
	char *method_name;
	char *resource;
	ptr = strtok_r(buff,"\r\n",&saveptr);
	method_name = strtok_r(ptr," ",&saveptr); //ptr will have the html method - ptr conterrà il metodo html
	resource = strtok_r(NULL," ",&saveptr);
	printf("Ho fatto le strtok %s\n",ptr);
	fflush(stdout);
	if (strcmp(method_name,"GET") == 0) {
		//if (strcmp(resource,"/favicon.ico") == 0) {
		//	return EXIT_SUCCESS;
		//}
		FILE *image = fopen("404.html","r");
		char *type = "text/html";
		char *path;
		path = malloc((strlen(resource) + 1)*sizeof(char));
		if (path == NULL){
			perror("malloc");
			return EXIT_FAILURE;
		}
		if (strcmp(resource,"/") != 0) {
			if (fclose(image) == -1) {
				perror("closedwew");
				free(buff);
				free(path);
				return EXIT_FAILURE;
			}
			sprintf(path,".%s",resource);
			printf("%s",path);
			fflush(stdout);
			image = fopen(path,"r");
			if (image == NULL) {
				perror("fopen");
				free(buff);
				free(path);
				return EXIT_FAILURE;
			}
			type = "image/jpeg";
		}

		printf("Bella zio\n");
		fflush(stdout);
		
		unsigned long fileLen;
		fseek(image, 0, SEEK_END);
        fileLen=ftell(image);
        fseek(image, 0, SEEK_SET);
		char *data;
		data = malloc(fileLen);
		if (data == NULL) {
			perror("malloc");
			free(data);
			free(buff);
			free(path);
			return EXIT_FAILURE;
		}
		printf("Pronti alla fine\n");
		fflush(stdout);
		
		fread(data,fileLen,1,image);

		fclose(image);

		printf("sasa %d\n",strlen(data));
		fflush(stdout);
		char *response;
		response = malloc(fileLen + strlen(type)*sizeof(char) + strlen("HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContentLength: %d\r\n\r\n%s"));
		if (response == NULL) {
			perror("malloc");
			free(response);
			free(data);
			free(buff);
			free(path);
			return EXIT_FAILURE;
		}
		printf("Qua ci sono\n");
		fflush(stdout);
		sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContentLength: %d\r\n\r\n%s",type,fileLen,data);
		printf("Eccolo: %s",response);
		fflush(stdout);
		writen = send(connsd,response,strlen(response),MSG_DONTWAIT);

		if (writen == 0) {
			perror("write");
			free(response);
			free(data);
			free(buff);
			free(path);
			return EXIT_FAILURE;
		}

		free(response);
		free(path);
		free(data);
		//chiama funzione GET
	}
	else if (strcmp(method_name,"HEAD") == 0) {
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
			free(buff);
			return EXIT_FAILURE;
		}
		printf("Ho scritto\n");
		fflush(stdout);
	}
	printf("Chiudo\n");
	fflush(stdout);
	if (shutdown(connsd,SHUT_RDWR) < 0) {
		perror("shutdown");
		free(buff);
		return EXIT_FAILURE;
	}
	printf("Shutdown fatto\n");
	fflush(stdout);
	if (close(connsd) < 0) {
		perror("close");
		free(buff);
		return EXIT_FAILURE;
	}
	printf("Tutto fatto\n");
	fflush(stdout);
	free(buff);
	return EXIT_SUCCESS;
}
