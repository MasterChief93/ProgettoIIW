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

/* Include the required headers from httpd */
#include "httpd.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"

/* Define prototypes of our functions in this module */
static void register_hooks(apr_pool_t *pool);
static int example_handler(request_rec *r);

/* Define our module as an entity and assign a function for registering hooks  */
module AP_MODULE_DECLARE_DATA   iiw_module =
{ 
    STANDARD20_MODULE_STUFF,
    NULL,//create_dir_conf, /* Per-directory configuration handler */
    NULL,//merge_dir_conf,  /* Merge handler for per-directory configurations */
    NULL,//create_svr_conf, /* Per-server configuration handler */
    NULL,//merge_svr_conf,  /* Merge handler for per-server configurations */
    NULL,//directives,      /* Any directives we may have for httpd */
    register_hooks   /* Our hook registering function */
}; 

/* register_hooks: Adds a hook to the httpd process */
static void register_hooks(apr_pool_t *pool)
{
    /* Create a hook in the request handler, so we get called when a request arrives */
    ap_hook_handler(example_handler, NULL, NULL, APR_HOOK_LAST);
}

static int example_handler(request_rec *r)
{
    /* First off, we need to check if this is a call for the "example" handler.
     * If it is, we accept it and do our things, it not, we simply return DECLINED,
     * and Apache will try somewhere else.
     */
    if (!r->handler || strcmp(r->handler, "example-handler")) return (DECLINED);
    
    char *orig = "./orig";
    char * modif = "./modif";
    
    /*INFORMATION GATHERING BEGIN*/

		char *saveptr;  
		char *saveptr2;    			//They are useful for the strtok_r (thread-safe strtok)
		char *saveptr3;
		char user_agent_line;		//It will contain the whole "User Agent" line in the header
		char *user_agent_intro;		//It will contain "User Agent: " string in order to find the right line in the header
		char user_agent[512];			//It will contain the User Agent string that we need
		char rline_copy[100];			//It will contain the request line previously obtained in order to use it inside the log
		char method_name[10];			//GET, HEAD, etc.
		char resource[256];				//The resource requested ("/","/favicon.ico",etc.)

		const apr_array_header_t    *fields;
		int                         i;
		apr_table_entry_t           *e = 0;
		
		fields = apr_table_elts(r->headers_in);
		e = (apr_table_entry_t *) fields->elts;
		for(i = 0; i < fields->nelts; i++) {
			if (strcmp("User-Agent", e[i].key);  // :?
			strcopy(user_agent,e[i].val);	
		}
			
		strcopy(method_name,r->method);		
		strcopy(resource,r->filename);							//resource will have the resource file name - resource contiene il nome della risorsa
		
		
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
			return METHOD_NOT_ALLOWED;
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
			
			                           	
				char resolution[128]; //It will contains the resolution of the client in the form of "decimal SPACE decimal"

				
					//libwurfl is called and the useragent with the resolution will be added on the db
					wurfl_interrogation(user_agent, resolution);
					
				
				
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

				
				
					
					resizing(path,new_path,width,height);	//I resize it with the new width and height

					image = open(new_path,O_RDWR);
					type = "image/jpeg";

				
			
				//if the image is on the database and on the disk, go on
		}
		
		//Those struct and variable will be used to obtain the length of the image

		unsigned long fileLen;
		struct stat fileStat;


		// fcntl(fileno(image),F_SETLKW,&lock);
		if (flock(image,LOCK_EX) == -1) {
		 	perror("lock image lock");
		 	sqlite3_close(db);
		 	shutdown_sequence(connsd);
		 	return EXIT_FAILURE;
		}

		if(fstat(image,&fileStat) < 0) {
         	perror("fstat image");
         	sqlite3_close(db);
         	shutdown_sequence(connsd);
         	return EXIT_FAILURE;
        }

        fileLen = fileStat.st_size;

		if (flock(image,LOCK_UN) == -1) {
		 	perror("lock image unlock");
		 	sqlite3_close(db);
		 	shutdown_sequence(connsd);
		 	return EXIT_FAILURE;
		}
		/*
		//The response will contain the HTTP HEADER string of response, of course
		char response[200];
		ssize_t resp_length;
		if (flag == 0) {
		  	resp_length = sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\nKeep-Alive: timeout=10\r\nConnection: Keep-Alive\r\n\r\n",type,fileLen);
		} else {
			resp_length = sprintf(response,"HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\nContent-Length: %d\r\nKeep-Alive: timeout=10\r\n,Connection: Keep-Alive\r\n\r\n",type,fileLen);
		}
		printf("resp_length = %ld\n",resp_length);
		fflush(stdout);
				

		int optval;
		// Enable TCP_CORK option on 'sockfd' - subsequent TCP output is corked
		//until this option is disabled. 
		optval = 1;
		setsockopt(connsd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));

		//cicle for writing the response on the socket
		ssize_t write_resp = 0;
		ssize_t len_resp = resp_length;
		int move = 0;
		
		while (len_resp > 0) {
			write_resp = send(connsd,&response[move],len_resp,MSG_DONTWAIT);
			if (write_resp == -1) continue;
			move += write_resp;
			len_resp -= write_resp;
		}
		*/
		
		 ap_set_content_type(r, type);
		 
		 
		//cicle for writing the image (or the page) on the socket only if the request is a GET!
		ssize_t  writen = 0;
		size_t nleft=fileLen;
		
		while(nleft > 0) {
			if ((writen = ap_rwrite(image, nleft, r)) > 0) { 
				nleft -= writen;
				image += writen;
				else{
					return HTTP_INTERNAL_SERVER_ERROR;
				}
		}


		
		printf("Ho finito. Il mio file descritor era: %d e ricomincio %lld\n",connsd,pthread_self());
		fflush(stdout);	
	}
}
