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
#include "parsing.h"
#include "resizing.h"

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

int Thread_Work(int connsd, int fdl, sqlite3 *db, char *orig, char *modif)
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
			printf("Sto dentro\n");
			fflush(stdout);
			if ((readn = recv(connsd, ptr, nleft, MSG_DONTWAIT)) < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					if (strlen(buff) == 0) {
						printf("e 0 ma continuo\n");
						fflush(stdout);
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
			free(buff);
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		//FINE LETTURA RICHIESTA

		//RICAVO LA RISORSA RICHIESTA
		char *saveptr;
		char *saveptr2;
		char *saveptr3;
		char *request_line;
		char *user_agent_line;
		char *user_agent_intro;
		char *user_agent;

		// user_agent = malloc(256*sizeof(char));
		// if (user_agent == NULL) {
		// 	perror("malloc");
		// 	shutdown_sequence(connsd);
		// 	return EXIT_FAILURE;
		// }
		char *method_name;
		char *resource;
		char *temp;


		request_line = strtok_r(buff,"\r\n",&saveptr);

		int i;

		for (i = 0; i < 5; i++) {
			user_agent_line = strtok_r(NULL,"\r\n",&saveptr);
			user_agent_intro = strtok_r(user_agent_line," ",&saveptr3);
			if (strcmp(user_agent_intro,"User-Agent:") == 0) break;
		}

		method_name = strtok_r(request_line," ",&saveptr2); 				//ptr will have the html method - ptr conterrà il metodo html
		resource = strtok_r(NULL," ",&saveptr2);  							//resource will have the resource file name - resource contiene il nome della risorsa

		user_agent = strtok_r(NULL,"",&saveptr3);   //it works...

		if (strcmp(method_name,"GET") == 0) {
			if (strcmp(resource,"/favicon.ico") == 0) {
				free(buff); 
				continue; //There is no favicon, so if the request is a GET of a favicon it will be ignored
			}

			//If it is not, the deafult page will be opened
			//These variables will be used in both cases
		FILE *image = NULL;
		char *type = NULL;
		

		int flag = 0;

		// In of a non-specific resource request

		if (strcmp(resource,"/") == 0) {
			image = fopen("default.html","r");
			type = "text/html";
			free(buff);
		} else { // in case of a specific resource request
			
			//TODO il path immagine totale includerà il "punto (.)" iniziale, la cartella prelevata dal config e il nome dell'immagine specificato nella richiesta HTTP

			char *path;

			path = malloc((256)*sizeof(char));
			if (path == NULL) {
				perror("malloc");
				free(buff);
				shutdown_sequence(connsd);
				return EXIT_FAILURE;
			}

			sprintf(path,"%s%s",orig,resource);			// adding the dot in order to use fopen

			int ispresent = dbcontrol(db,resource,1);

			if (ispresent == 0) { 					// if the image is not in the database 
				image = fopen("404.html","r");		// 404 error will be returned
				type = "text/html";
				flag = 1;
				if (image == NULL) {  				// If the opening of the 404.html page fails everything will be close
					perror("fopen");
					free(buff);
					free(path);
					shutdown_sequence(connsd);
					return EXIT_FAILURE;
				}
			} else {                            	// if the image is one the database
				char *resolution;
				resolution = malloc(128*sizeof(char));
				if (resolution == NULL) {
					free(buff);
					free(path);
					shutdown_sequence(connsd);
					return EXIT_FAILURE;
				}

				strcpy(resolution,dbfindUA(db,user_agent));

				if (strcmp(resolution,"NULL") == 0) {
					wurfl_interrogation(user_agent, resolution);
					dbaddUA(db,user_agent,resolution);
				}
				//CONTROLLO A CHE RISOLUZIONE E RICHIESTA
				// CONTROLLO SE GIA ESISTE A QUELLA RISOLUZIONE con dbcheck (Se non c'è la inserisce da solo) 0 se non c'è (modifico con image magick) o 1 se c'è (e vado diretto al percorso delle pagine)
				int width;
				int height;
				sscanf(resolution,"%d %d ",&width,&height);
				
				free(resolution);

				char *n_image;
				char *ext;
				// n_image = malloc(256*sizeof(char));
				// if (n_image == NULL) {
				// 	free(buff);
				// 	free(path);
				// 	shutdown_sequence(connsd);
				// 	return EXIT_FAILURE;
				// }
				
				// ext = malloc(5*sizeof(char));
				// if (ext == NULL) {
				// 	free(buff);
				// 	free(path);
				// 	shutdown_sequence(connsd);
				// 	return EXIT_FAILURE;
				// }
				char *saveptr4;
				n_image = strtok_r(resource,".",&saveptr4);
				ext = strtok_r(NULL,".",&saveptr4);

				//Si potrebbero invertire le malloc dato che new_path comprende new_image_name
				char *new_path;
				new_path = malloc((strlen(modif) + strlen(n_image) + 14)*sizeof(char));
				if (new_path == NULL) {
					free(buff);
					free(path);
					free(new_path);
					shutdown_sequence(connsd);
					return EXIT_FAILURE;
				}
				char *new_image_name;
				new_image_name = malloc((strlen(n_image) + 14)*sizeof(char));
				if (new_image_name == NULL) {
					free(buff);
					free(path);
					free(new_path);
					free(new_image_name);
					shutdown_sequence(connsd);
					return EXIT_FAILURE;
				}

				sprintf(new_image_name,"%s_%d_%d.%s",n_image,width,height,ext);
				sprintf(new_path,"%s%s",modif,new_image_name);

				int ischeck = dbcheck(db,new_image_name,resource);
				//check if the resized image entry is on the database
				if (ischeck == 0) {
					resizing(path,new_path,width,height);	//if it is not, I resize it
				}
				image = fopen(new_path,"r");
				type = "image/jpeg";
				if (image == NULL) {				// but if it is not on the disk
					dbremove(db,new_image_name,0);	// I remove the image entry from the db
					image = fopen("404.html","r");	// 404 will be returned
					type = "text/html";
					flag = 1;
					if (image == NULL) {  			// If the opening of the 404.html page fails everything will be close
						perror("fopen");
						free(buff);
						free(path);
						free(new_path);
						free(new_image_name);
						shutdown_sequence(connsd);
						return EXIT_FAILURE;
					}
				}
						
				free(buff);
				
				free(path);
			
				free(new_image_name); //Prima si libera new_image_name e poi new_path...
				
				free(new_path);
				
				
				
				//if the image is on the database and on the disk
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
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		char *data_copy; //In order to free the memory because data will be used and moved
		data_copy = data;

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
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		char *resp_copy;
		resp_copy = response;
		if (flag == 0) {
			sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContentLength: %d\r\n\r\n",type,fileLen);
		} else {
			sprintf(response,"HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\nContentLength: %d\r\n\r\n",type,fileLen);
		}
	
		ssize_t write_resp = 0;
		ssize_t len_resp = strlen(response);
		while (len_resp > 0) {
			write_resp = send(connsd,response,len_resp,MSG_DONTWAIT);
			if (write_resp == -1) continue;
			response += write_resp;
			len_resp -= write_resp;
		}
	

		// if (writen == 0) {
		// 	perror("write");
		// 	free(response);
		// 	free(data);
		// 	free(buff);
		// 	free(path);
		// 	shutdown_sequence(connsd);
		// 	return EXIT_FAILURE;
		// }
		
		ssize_t write_data;
		ssize_t len_data = fileLen;
		while (len_data > 0) {
			write_data = send(connsd,data,len_data,MSG_DONTWAIT);
			if (write_data == -1) continue;
			printf("Ho scritto %d\n",write_data);
			fflush(stdout);
			data += write_data;
			len_data -= write_data;
		}

		// if (writen == 0) {
		// 	perror("write");
		// 	free(response);
		// 	free(data);
		// 	free(buff);
		// 	free(path);
		// 	shutdown_sequence(connsd);
		// 	return EXIT_FAILURE;
		// }


		free(resp_copy);
		free(data_copy);

	} else if (strcmp(method_name,"HEAD") == 0) {
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
	}
	return shutdown_sequence(connsd);
}
