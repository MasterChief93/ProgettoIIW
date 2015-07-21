#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sqlite3.h>

#include "threadwork.h"    //Nostro
#include "processwork.h"
#include "db.h"

#define BUFF_SIZE 1024


int shutdown_sequence(int connsd) {
	if (shutdown(connsd,SHUT_RDWR) < 0) {
		perror("shutdown");
		return EXIT_FAILURE;
	}

	if (close(connsd) < 0) {
		perror("close");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Thread_Work(int connsd, int fdl, sqlite3 *db)
{
	//printf("Sono entrato nel Thread Work! Sono il tid %lld figlio di %lld\n",(long long int) pthread_self(), (long long int) getpid());
	//fflush(stdout);

	//READING SEQUENCE
	for (;;) {
		ssize_t readn, writen;
		size_t nleft;
		char *buff;
		buff = malloc(sizeof(char) * BUFF_SIZE);
		if (buff == NULL) {
			perror("malloc");
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		char *ptr = buff;
		errno = 0;
		nleft = BUFF_SIZE;
		while(nleft > 0) {
			if ((readn = recv(connsd, ptr, nleft, MSG_DONTWAIT)) < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					if (strlen(buff) == 0) {
						continue;
					} else {
						*ptr = '\0';
						break;
					}
				}
				else if (errno == EINTR) 
					readn = 0;
				else {
					free(buff);
					shutdown_sequence(connsd);
					return EXIT_FAILURE;
				}
			}

			else if (readn == 0) 
				break;

			if (strlen(buff) >= 1) {
				if (buff[strlen(buff)-1] == '\0') 
					break;
			}
			
			nleft -= readn;
			ptr += readn;
		}
		printf("La lunghezza di buff e: %d\n",strlen(buff));
		fflush(stdout);		
		if (buff[strlen(buff)-1] != '\0') {
			buff[strlen(buff)-1] = '\0';
		}
		errno = 0;
		if (strlen(buff) < 1) {
			perror("No string");
			printf("Non ho piu nulla da leggere\n");
			fflush(stdout);
			free(buff);
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		//FINE LETTURA RICHIESTA

		//RICAVO LA RISORSA RICHIESTA
		char *saveptr;
		char *method_name;
		char *resource;
		ptr = strtok_r(buff,"\r\n",&saveptr);
		method_name = strtok_r(ptr," ",&saveptr); //ptr will have the html method - ptr conterrà il metodo html
		resource = strtok_r(NULL," ",&saveptr);  //resource will have the resource file name - resource contiene il nome della risorsa

		if (strcmp(method_name,"GET") == 0) {
			if (strcmp(resource,"/favicon.ico") == 0) {
				free(buff); 
				continue; //There is no favicon, so if the request is a GET of a favicon it will be ignored
			}

			//If it is not, the deafult page will be opened
			//These variables will be used in both cases
			FILE *image = NULL;
			char *type = NULL;
			char *path;
			path = malloc((strlen(resource) + 1)*sizeof(char));
			if (path == NULL) {
				perror("malloc");
				free(buff);
				shutdown_sequence(connsd);
				return EXIT_FAILURE;
			}

			int flag = 0;
			// In of a non-specific resource request
			if (strcmp(resource,"/") == 0) {
				image = fopen("default.html","r");
				type = "text/html";
				
			}
			// in case of a specific resource request
			else {
				sprintf(path,".%s",resource); // adding the dot in order to use fopen
				image = fopen(path,"r");
				type = "image/jpeg";
				if (image == NULL) {
					image = fopen("404.html","r");
					type = "text/html";
					flag = 1;
					if (image == NULL) {  //If the opening of the 404.html page fails everything will be close
						perror("fopen");
						free(buff);
						free(path);
						shutdown_sequence(connsd);
						return EXIT_FAILURE;
					}
				}
			}

			unsigned long fileLen;
			struct stat fileStat;

			if(fstat(fileno(image),&fileStat) < 0)    
	        	return 1;
	        fileLen = fileStat.st_size;

			char *data;
			data = malloc(fileLen);
			if (data == NULL) {
				perror("malloc");
				free(data);
				free(buff);
				free(path);
				shutdown_sequence(connsd);
				return EXIT_FAILURE;
			}


			ssize_t red, n;
			n = fileLen;
			while (n > 0) {
				red = fread(data,1,n,image);
				n -= red;
				printf("%d %d\n",n,red);
				fflush(stdout);
			}

			fclose(image);


			char *response;
			if (flag == 0) {
				response = malloc(sizeof(char)*(strlen(type)* + strlen("HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContentLength: %d\r\n\r\n%s")));
			} else {
				response = malloc(sizeof(char)*(strlen(type)* + strlen("HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\nContentLength: %d\r\n\r\n%s")));
			}
			if (response == NULL) {
				perror("malloc");
				free(response);
				free(data);
				free(buff);
				free(path);
				shutdown_sequence(connsd);
				return EXIT_FAILURE;
			}

			if (flag == 0) {
				sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContentLength: %d\r\n\r\n",type,fileLen);
			} else {
				sprintf(response,"HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\nContentLength: %d\r\n\r\n",type,fileLen);
			}
		
			writen = send(connsd,response,strlen(response),MSG_DONTWAIT);

			if (writen == 0) {
				perror("write");
				free(response);
				free(data);
				free(buff);
				free(path);
				shutdown_sequence(connsd);
				return EXIT_FAILURE;
			}

			writen = send(connsd,data,fileLen,MSG_DONTWAIT);

			if (writen == 0) {
				perror("write");
				free(response);
				free(data);
				free(buff);
				free(path);
				shutdown_sequence(connsd);
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
			writen = send(connsd,"HTTP/1.1 405 Method Not Allowed\r\n\r\n",strlen("HTTP/1.1 405 Method Not Allowed\r\n\r\n"),MSG_DONTWAIT);

			if (writen == 0) {
				perror("write");
				free(buff);
				shutdown_sequence(connsd);
				return EXIT_FAILURE;
			}

		}
		free(buff);
	}
	return shutdown_sequence(connsd);
}
