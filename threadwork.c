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

#include "threadwork.h"    
#include "processwork.h"
#include "db.h"
#include "parsing.h"
#include "resizing.h"

#define BUFF_SIZE 3000


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

int Thread_Work(int connsd, int fdl, char *orig, char *modif, int test)
{
	/*READING SEQUENCE BEGIN*/
	sqlite3 *db;
	if (sqlite3_open("db/images.db", &db)) {  //Open the conection to the database - Apre la connessione al database
		perror("error in sqlite_open");
		sqlite3_close(db);                    //In any case of server shutdown, close the db connection first - In ogni caso di chiusura del server, chiude anche la connessione al database 
		return EXIT_FAILURE;
	}

	for (;;) {
		ssize_t readn = 0, writen = 0;
		size_t nleft;
		int method_flag = 0;

		char logstring[1024];

		// Client IP Gathering
		struct sockaddr_in addr;
    	socklen_t addr_size = sizeof(struct sockaddr_in);
    	int ip = getpeername(connsd, (struct sockaddr *)&addr, &addr_size);
    	char clientip[20];
    	strcpy(clientip,inet_ntoa(addr.sin_addr));

		char buff[BUFF_SIZE];
		
		memset(buff,0,BUFF_SIZE);

		char *ptr = buff;

		errno = 0;

		nleft = BUFF_SIZE;

		while(nleft > 0) {
			if ((readn = recv(connsd, ptr, nleft, 0)) > 0) { 
				nleft -= readn;
				ptr += readn;
				if (*(ptr-2) == '\r' && *(ptr-1) == '\n') {
					break;
				} else  {
					continue;
				}
			} else if (readn == 0) {
				break;
			} else if (errno != 0 || readn == -1) {
				perror("buff empty or an error occurred");
				while (sqlite3_close(db) != SQLITE_OK);
				shutdown_sequence(connsd);
				return EXIT_FAILURE;
			}
		}
	
		*ptr = '\0';
		ptr -= strlen(buff);
		errno = 0;

		//If really nothing has been read I shutdown the connect
		if (strlen(buff) == 0) {
			while (sqlite3_close(db) != SQLITE_OK);
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		/*READING SEQUENCE END*/

		/*INFORMATION GATHERING BEGIN*/

		char *saveptr;  
		char *saveptr2;    			//They are useful for the strtok_r (thread-safe strtok)
		char *saveptr3;
		char *saveptr4;
		char *saveptr5;
		char *saveptr6;

		char *request_line;			//It will contain the "GET / HTTP/1.1" string
		char *user_agent_line;		//It will contain the whole "User Agent" line in the header
		
		char *user_agent_intro;		//It will contain "User Agent: " string in order to find the right line in the header
		char *user_agent;			//It will contain the User Agent string that we need
		char rline_copy[100];			//It will contain the request line previously obtained in order to use it inside the log

		char *method_name;			//GET, HEAD, etc.
		char *resource;				//The resource requested ("/","/favicon.ico",etc.)
		
		char buff_copy[strlen(buff)];  //It will contain the copy of the buff, so we can use it to obtain the Accept header line (if exists)
		memset(buff_copy,0,strlen(buff));
		strcpy(buff_copy,buff);

		request_line = strtok_r(buff,"\r\n",&saveptr);
		strcpy(rline_copy,request_line);

		//The buff could be shortest than usual so the request_line couldn't even exists
		if (request_line == NULL) {
			perror("Not valid request");
			shutdown_sequence(connsd);
			while (sqlite3_close(db) != SQLITE_OK);
			return EXIT_FAILURE;
		}

		int i;

		//I'll cicle through the line of the header until the User-Agent line
		for (;;) {
			user_agent_line = strtok_r(NULL,"\r\n",&saveptr);
			if (user_agent_line != NULL) {
				user_agent_intro = strtok_r(user_agent_line," ",&saveptr3);
				if (user_agent_intro != NULL) {
					if (strcmp(user_agent_intro,"User-Agent:") == 0) break;
				} else break;
			} else break;
		}

		method_name = strtok_r(request_line," ",&saveptr2); 				
		resource = strtok_r(NULL," ",&saveptr2);  							//resource will have the resource file name - resource contiene il nome della risorsa
		user_agent = strtok_r(NULL,"",&saveptr3);   						
		
		char *accept_line;    //It will contain the whole accept line in the header
		char *accept_intro;	  //It will contain the string "Accept:" in order to find the right line
		char *accept;		  //It will contain the value of the Accept line
		

		//I'll cicle through the line of the header until the Accept line
		strtok_r(buff_copy,"\r\n",&saveptr);
		for (;;) {
			accept_line = strtok_r(NULL,"\r\n",&saveptr);
			if (accept_line != NULL) {
				accept_intro = strtok_r(accept_line," ",&saveptr3);
				if (accept_intro != NULL) {
					if (strcmp(accept_intro,"Accept:") == 0) break;
				} else break;
			} else break;
		}

		
		char *results; //Will be used to contain the resources specified inside the Accept line
		char *value;   //It will contain the specified quality (E.g. "q=0.9")
		char *temp;	   
		float quality = 0;  //It will contain the float value of the quality or 0 if none is present

		//If accept_intro is NULL this algorithm will be ignored
		if (accept_intro != NULL) {
			accept = strtok_r(NULL,"",&saveptr3); 	//accept will contain the whole string 
			if (accept != NULL) {
				//result will contain the resources (e.g: "*/*" or "images/jpeg") and it will sometime contain the q=X value
				while ((results = strtok_r(accept,",",&saveptr4)) != NULL) {	
					if ((temp = strtok_r(results,";",&saveptr5)) != NULL) { //temp is the part before the semicolumn and the q=X value
						if (strcmp(temp,"*/*") == 0 || strcmp(temp,"image/jpeg") == 0 || strcmp(temp,"image/*") == 0) {
							value = strtok_r(NULL,";",&saveptr5); //value will be the q=X string
							break;
						}
					} else break;
					accept = NULL;
				}
			} else break;
			//If value has been gathered in the previous cycle then I will make it a float
			if (value != NULL) {
				if (strtok_r(value,"=",&saveptr6) != NULL) {
					quality = strtof(strtok_r(NULL,"=",&saveptr6),NULL);
				}
			}
		}



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
			writen = send(connsd,"HTTP/1.1 405 Method Not Allowed\r\n\r\n",strlen("HTTP/1.1 405 Method Not Allowed\r\n\r\n"),0);
			if (writen == 0) {
				perror("write");
				while (sqlite3_close(db) != SQLITE_OK);
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

		int image = -1;				//Will contain the HTML page or the image
		char *type = NULL;			//It will contain the type string that has to be part of HTTP Header
		

		int flag = 0;			//This flag will be used later in order to response with a 404 page or the page requested
								// 1 for 404, 0 for 200
		
		// In of a non-specific resource request the default.html page will be returned ("text/html" type)
		if (strcmp(resource,"/") == 0) {
			image = open("default.html",O_RDWR);
			type = "text/html";

		// in case of a specific resource request
		} else { 
			if (test == 1) {
				char name[strlen(resource) + 2];
				sprintf(name,"./orig/%s",resource);
				type = "image/jpeg";
				if ((image = open(name,O_RDWR)) == -1) {
					image = open("404.html",O_RDWR);		// 404 error page will be returned
					type = "text/html";
				}
			} else {
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
						while (sqlite3_close(db) != SQLITE_OK);
						shutdown_sequence(connsd);
						return EXIT_FAILURE;
					}

				// if the image is one the database instead
				} else {                            	

					char resolution[128]; //It will contains the resolution of the client in the form of "decimal SPACE decimal"
					int width;
					int height;
					
					//if no quality value has been found I will gather the resolution informations
					if (quality == 0) {

						//The result of dbfindUA will be copied in resolution if the user-agent is already on the db
						strcpy(resolution,dbfindUA(db,user_agent));   

						printf("resolution = %s\n",resolution);
						fflush(stdout);
						//If resolution has not be filled by dbfindUA so there is not an entry in the db
						if (strcmp(resolution,"NULL") == 0) {
							//libwurfl is called and the useragent with the resolution will be added on the db
							wurfl_interrogation(user_agent, resolution);
							dbaddUA(db,user_agent,resolution);
						}
					
						//From resolution width and height will be parsed and stored in two different integer variables
						
						sscanf(resolution,"%d %d ",&width,&height);
					}


					char *n_image;
					char *ext;
		
					char *saveptr4;
					n_image = strtok_r(resource,".",&saveptr4);				//n_image will contains just the name of the image
					ext = strtok_r(NULL,".",&saveptr4);						//ext the extension

					char new_path[strlen(modif)+strlen(n_image)+14];
					
					char new_image_name[strlen(n_image)+14];

					//The name of the modified image will be different if the quality is specified
					if (quality == 0) {
						sprintf(new_image_name,"%s_%d_%d.%s",n_image,width,height,ext);    //new_image_name will contain the complete name of the resized image
						sprintf(new_path,"%s%s",modif,new_image_name);					   //new_path will be the relative path of the modified image
					} else {
						sprintf(new_image_name,"%s_%.2f.%s",n_image,quality,ext);    //new_image_name will contain the complete name of the resized image
						sprintf(new_path,"%s%s",modif,new_image_name);
					}
					
					
					while (image == -1) {
						//dbcheck will control if an entry of the resized image already exists
						int ischeck = dbcheck(db,new_image_name,resource);
						//if the image is not on the db
						if (ischeck == 0) {
							resizing(path,new_path,width,height,quality);	//I resize it with the new width and height or the quality specified
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
			}//if the image is on the database and on the disk, go on			
		}
		

		//Those struct and variable will be used to obtain the length of the image
		time_t t;
		struct tm *tmp;
		struct tm *result;

		t = time(NULL);
		tmp = localtime_r(&t,&result);
		if (tmp == NULL) {
   			perror("localtime");
   			while (sqlite3_close(db) != SQLITE_OK);
   			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}

		char timestring[30];
		if (strftime(timestring,sizeof(timestring),"%d/%b/%Y:%H:%M:%S %z",tmp) == 0) {
			perror("strftime");
			while (sqlite3_close(db) != SQLITE_OK);
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		unsigned long fileLen;
		struct stat fileStat;


		if (flock(image,LOCK_EX) == -1) {
		 	perror("lock image lock");
		 	while (sqlite3_close(db) != SQLITE_OK);
		 	shutdown_sequence(connsd);
		 	return EXIT_FAILURE;
		}

		if(fstat(image,&fileStat) < 0) {
         	perror("fstat image");
         	while (sqlite3_close(db) != SQLITE_OK);
         	shutdown_sequence(connsd);
         	return EXIT_FAILURE;
        }

        //fileLen will contain the size of the image or the page on the disk
        fileLen = fileStat.st_size;

		if (flock(image,LOCK_UN) == -1) {
		 	perror("lock image unlock");
		 	while (sqlite3_close(db) != SQLITE_OK);
		 	shutdown_sequence(connsd);
		 	return EXIT_FAILURE;
		}

		
		
		//The response will contain the HTTP HEADER string of response, of course
		char response[200];
		ssize_t resp_length;
		if (flag == 0) {
		  	resp_length = sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\nKeep-Alive: timeout=10\r\nConnection: Keep-Alive\r\n\r\n",type,fileLen);
		} else {
			resp_length = sprintf(response,"HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\nContent-Length: %d\r\nKeep-Alive: timeout=10\r\n,Connection: Keep-Alive\r\n\r\n",type,fileLen);
		}
				

		int optval;
		/* Enable TCP_CORK option on 'sockfd' - subsequent TCP output is corked
		until this option is disabled. */
		optval = 1;
		setsockopt(connsd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));

		//cicle for writing the response on the socket
		ssize_t write_resp = 0;
		ssize_t len_resp = resp_length;
		int move = 0;
		while (len_resp > 0) {
			write_resp = send(connsd,&response[move],len_resp,0);
			if (write_resp == -1) continue;
			move += write_resp;
			len_resp -= write_resp;
		}
		
		//cicle for writing the image (or the page) on the socket only if the request is a GET!
		ssize_t numsend;
		if ((numsend = sendfile(connsd,image,0,fileLen)) == -1) {
			perror("sendfile");
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		close(image);
		

		optval = 0;
		setsockopt(connsd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));


		if (lockf(fdl, F_LOCK,0) == -1) {
			perror("lockf fdl");
			while (sqlite3_close(db) != SQLITE_OK);
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
		//In order to write on the access log file
		if (flag == 0) dprintf(fdl,"%s - - %s %s 200 %d\n",clientip,timestring,rline_copy,fileLen);
		else dprintf(fdl,"%s - - %s %s 404 %d\n",clientip,timestring,rline_copy,fileLen);

		if (lockf(fdl, F_ULOCK,0) == -1) {
			perror("lockf fdl");
			while (sqlite3_close(db) != SQLITE_OK);
			shutdown_sequence(connsd);
			return EXIT_FAILURE;
		}
	}	
}