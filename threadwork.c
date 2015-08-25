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
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/sendfile.h>

#include "threadwork.h"    //Nostro
#include "processwork.h"
#include "db.h"
#include "parsing.h"
#include "resizing.h"

#define BUFF_SIZE 2000


int shutdown_sequence(int connsd) {
	if (shutdown(connsd,SHUT_RDWR) < 0) {
		perror("shutdown");
		return EXIT_FAILURE;
	}

	if (close(connsd) < 0) {
		perror("close");
		return EXIT_FAILURE;
	}
	printf("Ho chiuso la connessione\n");
	fflush(stdout);
	return EXIT_SUCCESS;
}

int Thread_Work(int connsd, int fdl, sqlite3 *db, char *orig, char *modif)
{
	/*READING SEQUENCE BEGIN*/
	for (;;) {
		ssize_t readn, writen;
		size_t nleft;
		int method_flag = 0;

		char logstring[1024];

		struct sockaddr_in addr;
    	socklen_t addr_size = sizeof(struct sockaddr_in);
    	int ip = getpeername(connsd, (struct sockaddr *)&addr, &addr_size);
    	char clientip[20];
    	strcpy(clientip,inet_ntoa(addr.sin_addr));


		char buff[BUFF_SIZE];
		
		//fd_set rfds; //in order to use the select() call

		char *ptr = buff;

		errno = 0;

		nleft = BUFF_SIZE;

		//FD_ZERO(&rfds);
	    //FD_SET(connsd, &rfds);
	    printf("sto servendo io: %lld\n",pthread_self());
	    fflush(stdout);
	    //if (select(connsd+1, &rfds, NULL, NULL, NULL)) {  						//If there is a socket avalaible for reading
		while(nleft > 0) {
			printf("Entering cylce and reading\n");
			fflush(stdout);
			if ((readn = recv(connsd, ptr, nleft, 0)) > 0) { 
				nleft -= readn;
				ptr += readn;
				printf("reading...\n");
				fflush(stdout);
			}
			if (readn == 0) {
				printf("connection closed or nothing more to read\n");
				fflush(stdout);
				break;
			}
			if (readn == -1) {
				printf("error occurred\n");
				fflush(stdout);
				break;
			}
		}
			//}
		//}
	 //    errno = 0;
	 //    while ((readn = recv(connsd, buff, BUFF_SIZE, MSG_DONTWAIT)) > 0) {
	 //    	if (errno == EAGAIN || errno == EWOULDBLOCK) {
	 //    		if (strlen(buff) == 0) {
		// 			errno = 0; 				//It is important to reset the errno!!
		// 			continue;
		// 					//If other things occured than I will terminate the string and exit the cicle	
		// 			} else break;
	 //    	}
	 //    } 
	   
		// if (readn == 0) {
		// 	printf("connection closed by the client\n");
		// 	fflush(stdout);

		// }
		// if(readn == -1) {
		// 	printf("recv failed\n");
		// 	fflush(stdout);

		// }
		buff[strlen(buff)-1] = '\0';
		printf("buff =\n%s\n",buff);
		fflush(stdout);
	    /*
	    //if (select(connsd+1, &rfds, NULL, NULL, NULL)) {  						//If there is a socket avalaible for reading
			while(nleft > 0) {

				//Check if it is true that it is avalaible
				//if (FD_ISSET(connsd,&rfds)) { 			  						
					
					//I will read as much as I can using the MSG_DONTWAIT flag making the call non-blocking
					//that means that or the call will succed or it will be closed by the other side
					if ((readn = recv(connsd, ptr, nleft, MSG_DONTWAIT)) < 0) { 

						//If the non-blocking recv fails, it could set errno with one of the following errorcode
						if (errno == EAGAIN || errno == EWOULDBLOCK) {

							//This check has been implemented due to an error that happened several times
							//The buffer was empty even if a new data was sent.
							//This check gives a sort of second chance to the recv.			
							if (strlen(buff) == 0) {
								//FD_SET(connsd,&rfds);
								errno = 0; 				//It is important to reset the errno!!
								continue;
							//If other things occured than I will terminate the string and exit the cicle	
							} else {
								*ptr = '\0';
								break;
							}
						// If the conenction has been closed by the client
						} else if (errno == EINTR) readn = 0;
						// If other things occured I will simply shutdown the connection
						else {
							shutdown_sequence(connsd);
							return EXIT_FAILURE;
						}
					// If I read nothing
					} else if (readn == 0) break;
					
					nleft -= readn;
					ptr += readn;
					printf("buff = %s\n",buff);
					fflush(stdout);
					//FD_SET(connsd,&rfds);
					errno = 0;
				}
			//}
		//}
		
		//It is a redudant check but it doesn't cost anything, so...
		if (buff[strlen(buff)-1] != '\0') {
			buff[strlen(buff)-1] = '\0';
		}
		*/
		errno = 0;

		//If really nothing has been read I shutdown the connect
		if (strlen(buff) == 0) {
			perror("No string");
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		/*READING SEQUENCE END*/

		/*INFORMATION GATHERING BEGIN*/

		char *saveptr;  
		char *saveptr2;    			//They are useful for the strtok_r (thread-safe strtok)
		char *saveptr3;
		char *request_line;			//It will contain the "GET / HTTP/1.1" string
		char *user_agent_line;		//It will contain the whole "User Agent" line in the header
		char *user_agent_intro;		//It will contain "User Agent: " string in order to find the right line in the header
		char *user_agent;			//It will contain the User Agent string that we need
		char rline_copy[100];			//It will contain the request line previously obtained in order to use it inside the log

		char *method_name;			//GET, HEAD, etc.
		char *resource;				//The resource requested ("/","/favicon.ico",etc.)

		request_line = strtok_r(buff,"\r\n",&saveptr);
		strcpy(rline_copy,request_line);
		//The buff could be shortest than usual so the request_line couldn't even exists
		if (request_line == NULL) {
			perror("Not valid request");
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}

		int i;

		//I'll cicle through the line of the header until the User-Agent line
		for (i = 0; i < 5; i++) {
			user_agent_line = strtok_r(NULL,"\r\n",&saveptr);
			user_agent_intro = strtok_r(user_agent_line," ",&saveptr3);
			if (strcmp(user_agent_intro,"User-Agent:") == 0) break;
		}

		method_name = strtok_r(request_line," ",&saveptr2); 				
		resource = strtok_r(NULL," ",&saveptr2);  							//resource will have the resource file name - resource contiene il nome della risorsa
		user_agent = strtok_r(NULL,"",&saveptr3);   						//it works...
		
		/*INFORMATION GATHERING END*/
		

		/*METHOD AND RESOURCE SWITCHING*/

		/* There are three different case:
		*  1. It is a GET request
		*  2. It is an HEAD request
		*  3. It is another request and it is unsupported
		*/


		if (strcmp(method_name,"GET") == 0) method_flag = 0;
		else if(strcmp(method_name,"HEAD") == 0) method_flag = 1;
		else {
			writen = send(connsd,"HTTP/1.1 405 Method Not Allowed\r\n\r\n",strlen("HTTP/1.1 405 Method Not Allowed\r\n\r\n"),MSG_DONTWAIT);
			if (writen == 0) {
				perror("write");
				shutdown_sequence(connsd);
				return EXIT_FAILURE;
			}
		}
		
		/* The resource requested with the GET or HEAD method can be of different type, so:
		*  1. /favicon.ico is requested
		*  2. The root ("/") is requested
		*  3. A generic resource is requested
		*/

		//There is no favicon, so if the request is a GET of a favicon it will be ignored
		//if (strcmp(resource,"/favicon.ico") == 0) return shutdown_sequence(connsd);

		//If it is not the favicon
		//These variables will be used in both cases

		int image = -1;			//Will contain the HTML page or the image
		char *type = NULL;			//It will contain the type string that has to be part of HTTP Header
		

		int flag = 0;			//This flag will be used later in order to response with a 404 page or the page requested
								// 1 for 404, 0 for 200
		
		// In of a non-specific resource request the default.html page will be returned ("text/html" type)
		if (strcmp(resource,"/") == 0) {
			image = open("default.html",O_RDWR);
			type = "text/html";

		// in case of a specific resource request
		} else { 

			//TODO il path immagine totale includerà il "punto (.)" iniziale, la cartella prelevata dal config e il nome dell'immagine specificato nella richiesta HTTP
			char path[256];	  //It will contains the path of the original image 
		
			sprintf(path,"%s%s",orig,resource);			// adding the dot in order to use fopen
			
			//We need to control if the original image is on the database 
			//There must be no discrepancy between the db and the file on the disk!

			int ispresent = dbcontrol(db,resource,1);


			if (ispresent == 0) { 					// if the image is not in the database 
				image = open("404.html",O_RDWR);		// 404 error page will be returned
				type = "text/html";
				flag = 1;							
				if (image == -1) {  				// If the opening of the 404.html page fails everything will be close
					perror("open");
					shutdown_sequence(connsd);
					return EXIT_FAILURE;
				}

			// if the image is one the database instead
			} else {                            	
				char resolution[128]; //It will contains the resolution of the client in the form of "decimal SPACE decimal"

				//The result of dbfindUA will be copied in resolution if the user-agent is already on the db
				strcpy(resolution,dbfindUA(db,user_agent));   
				printf("resolution = %s",resolution);
				fflush(stdout);
				//If resolution has not be filled by dbfindUA so there is not an entry in the db
				if (strcmp(resolution,"NULL") == 0) {
					//libwurfl is called and the useragent with the resolution will be added on the db
					wurfl_interrogation(user_agent, resolution);
					dbaddUA(db,user_agent,resolution);
				}
				
				// CONTROLLO SE GIA ESISTE A QUELLA RISOLUZIONE con dbcheck (Se non c'è la inserisce da solo) 0 se non c'è (modifico con image magick) o 1 se c'è (e vado diretto al percorso delle pagine)
				//From resolution width and height will be parsed and stored in two different integer variables
				int width;
				int height;
				sscanf(resolution,"%d %d ",&width,&height);
	

				char *n_image;
				char *ext;
	
				char *saveptr4;
				n_image = strtok_r(resource,".",&saveptr4);				//n_image will contains just the name of the image
				ext = strtok_r(NULL,".",&saveptr4);						//ext the extension

				//Si potrebbero invertire le malloc dato che new_path comprende new_image_name
				char new_path[strlen(modif)+strlen(n_image)+14];
				
				char new_image_name[strlen(n_image)+14];

				sprintf(new_image_name,"%s_%d_%d.%s",n_image,width,height,ext);    //new_image_name will contain the complete name of the resized image
				sprintf(new_path,"%s%s",modif,new_image_name);					   //new_path will be the relative path of the modified image

				
				
				while (image == -1) {
					//dbcheck will control if an entry of the resized image already exists
					int ischeck = dbcheck(db,new_image_name,resource);
		
					//if the image is not on the db
					if (ischeck == 0) {
						resizing(path,new_path,width,height);	//I resize it with the new width and height
					}

					image = open(new_path,O_RDWR);
					type = "image/jpeg";

					// but if it is not on the disk
					if (image == -1) {	
						dbremove(db,new_image_name,0);	// I remove the image entry from the db to prevent other discrepancy
						continue;
					}
				}
			}
				//if the image is on the database and on the disk, go on
		}
		

		//Those struct and variable will be used to obtain the length of the image
		time_t t;
		struct tm *tmp;
		struct tm *result;

		t = time(NULL);
			tmp = localtime_r(&t,&result);
			if (tmp == NULL) {
   			perror("localtime");
   			shutdown_sequence(connsd);
			return EXIT_FAILURE;
			}
		char timestring[30];
		if (strftime(timestring,sizeof(timestring),"%d/%b/%Y:%H:%M:%S %z",tmp) == 0) {
			perror("strftime");
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}

		unsigned long fileLen;
		struct stat fileStat;

		// struct flock lock;
		// lock.l_type = F_RDLCK;
  //   	   lock.l_start = 0;
  //    lock.l_whence = SEEK_SET;
  //    lock.l_len = 0;

		// fcntl(fileno(image),F_SETLKW,&lock);
		if (flock(image,LOCK_EX) == -1) {
		 	perror("lock image lock");
		 	shutdown_sequence(connsd);
		 	return EXIT_FAILURE;
		}

		if(fstat(image,&fileStat) < 0)    
         	return 1;
        fileLen = fileStat.st_size;

  //       char data[fileLen];		//The data string will be initialized
		// //char *data_copy; 
		// //data_copy = data;

		// //The only way to copy the data of the image inside the string data

  //      	ssize_t red, n;
		// n = fileLen;
		// while (n > 0) {
		// 	red = fread(data,1,n,fdopen(image,"r+"));
		// 	n -= red;
		// }

		// data = (char *) mmap(NULL,fileLen, PROT_READ,MAP_PRIVATE,fileno(image),0);
		// if (data == MAP_FAILED) {
		// 	perror("mmap");
		// 	shutdown_sequence(connsd);
		// 	return EXIT_FAILURE;
		// }

		// lock.l_type = F_UNLCK;
		// fcntl(fileno(image), F_SETLK, &lock);
		if (flock(image,LOCK_UN) == -1) {
		 	perror("lock image unlock");
		 	shutdown_sequence(connsd);
		 	return EXIT_FAILURE;
		}

		
		
		char *response;		//The response will contain the HTTP HEADER string of response, of course 

		//Flag will do its job
		if (flag == 0) {
			response = malloc(sizeof(char)*(strlen(type)* + strlen("HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContentLength: %d\r\nKeep-Alive: timeout=10\r\nConnection: Keep-Alive\r\n\r\n%s")));
		} else {
			response = malloc(sizeof(char)*(strlen(type)* + strlen("HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\nContentLength: %d\r\nKeep-Alive: timeout=10\r\n,Connection: Keep-Alive\r\n\r\n%s")));
		}
		if (response == NULL) {
			perror("malloc");
			free(response);
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}

		char *resp_copy;
		resp_copy = response;
		//The same here adding type and fileLen in the string
		if (flag == 0) {
			sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContentLength: %d\r\nKeep-Alive: timeout=10\r\nConnection: Keep-Alive\r\n\r\n",type,fileLen);
		} else {
			sprintf(response,"HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\nContentLength: %d\r\nKeep-Alive: timeout=10\r\nConnection: Keep-Alive\r\n\r\n",type,fileLen);
		}

		int optval;
		/* Enable TCP_CORK option on 'sockfd' - subsequent TCP output is corked
		until this option is disabled. */
		optval = 1;
		setsockopt(connsd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));

		//cicle for writing the response on the socket
		ssize_t write_resp = 0;
		ssize_t len_resp = strlen(response);
		while (len_resp > 0) {
			write_resp = send(connsd,response,len_resp,MSG_DONTWAIT);
			if (write_resp == -1) continue;
			response += write_resp;
			len_resp -= write_resp;
		}
		

		//cicle for writing the image (or the page) on the socket only if the request is a GET!
		ssize_t numsend;
		if ((numsend = sendfile(connsd,image,0,fileLen)) == -1) {
			perror("sendfile");
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		printf("%d\n",numsend);
		fflush(stdout);
		close(image);
		

		optval = 0;
		setsockopt(connsd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));

		// if (method_flag == 0) {
		// 	ssize_t write_data;
		// 	ssize_t len_data = fileLen;
		// 	int move = 0;
		// 	while (len_data > 0) {
		// 		write_data = send(connsd,&data[move],len_data,MSG_DONTWAIT);
		// 		if (write_data == -1) continue;
		// 		move += write_data;
		// 		len_data -= write_data;
		// 	}
		// }

		if (lockf(fdl, F_LOCK,0) == -1) {
			perror("lockf fdl");
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}

		if (flag == 0) dprintf(fdl,"%s - - %s %s 200 %d\n",clientip,timestring,rline_copy,fileLen);
		else dprintf(fdl,"%s - - %s %s 404 %d\n",clientip,timestring,rline_copy,fileLen);

		if (lockf(fdl, F_ULOCK,0) == -1) {
			perror("lockf fdl");
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		printf("Ho finito. Arrivederci\n");
		fflush(stdout);
		//return shutdown_sequence(connsd);
	//If the method is HEAD
	// } else if (strcmp(method_name,"HEAD") == 0) {
	// 	printf("Ho fatto le HEAD\n");
	// 	fflush(stdout);
		//chiama funzione HEAD

	//If the method is unsupported
	
	}	

}

