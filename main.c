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
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sqlite3.h>

#include "processwork.h"     //Nostro
#include "threadwork.h"
#include "fileman.h"
#include "db.h"
#include "garbage.h"
#include "menu.h"
#include "page_generator.h"


void sighandler(int sig) {
	printf("server shutdown\n");
	fflush(stdout);
	_exit(EXIT_FAILURE);
}

int main()
{
	int reuse,fde,fdc, sock, i, mem, dbmem, fdal,fdl;
	struct sockaddr_in servaddr;
	sem_t *semaphore;
	struct Config *cfg;


	if ((fde = open("error.log", O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1) {
		perror("Error in opening error.log");
		return (EXIT_FAILURE);
	}
	

	if (dup2(fde, STDERR_FILENO) == -1) {
		perror("Error in opening error.log");
		return (EXIT_FAILURE);
	}

	if (close(fde)==-1){
		perror("Error in closing error.log");
		return (EXIT_FAILURE);
	}
		
	if ((mem = shmget(IPC_PRIVATE, sizeof(struct Config), O_CREAT|0666)) == -1)
	{
		perror("Error in shmget");
		return (EXIT_FAILURE);
	}
	
	if ((cfg = shmat(mem, NULL, 0))==NULL)
	{
		perror("Error in shmat");
		return (EXIT_FAILURE);
	}	
	
	if ((fdc = open("config.ini", O_CREAT | O_EXCL| O_RDWR, 0666)) == -1) {  //Create config.ini, unless it already exists - Crea config.ini, a meno che già non esista
		if ((fdc = open("config.ini", O_RDWR)) == -1) {                      // If config.ini already exists, open it - Se config.ini già esiste, aprilo
			perror("Error in opening config.ini");
			return (EXIT_FAILURE);
		}
		if (Load_Config(fdc, cfg)==EXIT_FAILURE)                             //Load the values found on config.ini - Carica i valori presenti sul file config.ini
		{
			perror("Error in loading from config.ini");
			return (EXIT_FAILURE);
		}                                                  
	}
	
	else{
		if (Set_Config_Default(fdc, cfg)==EXIT_FAILURE)                      //If the file doesn't exists, initialize it to the default values - Se non esisteva, inizializza le voci ai valori di default
		{
			perror("Error in creating config.ini . Try deleting it manually.");
			return (EXIT_FAILURE);
		}                                          
	}
	
	while (start_menu(cfg) != 2523);
	page_generator();
	//Create_access_log_file;
	if ((fdal = open("access.log", O_CREAT | O_EXCL | O_APPEND | O_RDWR, 0666)) == -1) {  // Create access.log, unless it already exists - Crea log.log, a meno che già non esista
		if ((fdal = open("access.log", O_APPEND | O_RDWR)) == -1) {                      // If access.log already exists, open it - Se log.log già esiste, aprilo
			perror("Error in opening log.log");
			return (EXIT_FAILURE);
		}
	}

	//Create_log_file
	if ((fdl = open("log.log", O_CREAT | O_EXCL | O_APPEND | O_RDWR, 0666)) == -1) {  // Create log.log, unless it already exists - Crea log.log, a meno che già non esista
		if ((fdl = open("log.log", O_APPEND | O_RDWR)) == -1) {                      // If log.log already exists, open it - Se log.log già esiste, aprilo
			perror("Error in opening log.log");
			return (EXIT_FAILURE);
		}
	}

	switch (fork())                                                    //Creates a Process to keep under control the size of the cache - Crea un processo per mantenere sotto controllo la grandezza della cache
			{
				case -1:
					perror("Error in fork");
					exit(EXIT_FAILURE);
				case 0:
					Garbage_Collector(/*db,*/ cfg, fdl);
			}           
			

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) <0) {  
		perror("Error in socket");
		return (EXIT_FAILURE);
	}

	if (memset((void*)&servaddr, 0, sizeof(servaddr))==NULL)
	{
		perror("Error in memset");
		return (EXIT_FAILURE);
	}
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port=htons(cfg->Serv_Port);

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {        //To allow the reuse of the port in case of restart of the server - Per permettere il riuso della porta in caso di riavvio del server
		perror("Error in setsockopt");
		exit(EXIT_FAILURE);
	}

	struct timeval timeout;      
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0) {
		perror("Error in setsockopt");
		exit(EXIT_FAILURE);
  	}


	int optval = 1;
	socklen_t optlen = sizeof(optval);
	if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
		perror("setsockopt()");
		exit(EXIT_FAILURE);
	}

	if(bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
	{
		perror("Error in bind");
		return (EXIT_FAILURE);
	}
	
	if (listen(sock, SOMAXCONN)<0)   //SOMAXCONN = accept connections untli the SO gives up - SOMAXCONN = acccetta connessioni finchè il SO non getta la spugna
	{
		perror("Error in listen");
		return (EXIT_FAILURE);
	}

	int fdlock = open("lock",O_RDWR);
	if (fdlock == -1) {
		perror("open");
		return EXIT_FAILURE;
	}


	for (i=1; i <= cfg->Max_Prole_Num; i++)        //Preforking - Preforking
	{
		switch (fork())    //Funziona?
		{
			case -1:
				perror("Error in prefork");
				exit(EXIT_FAILURE);
			case 0:
				printf("Sono un figlio\n");
				fflush(stdout);
				Process_Work(sock, fdlock, cfg, fdal);//, db);  //Da aggiungere in un nostro header
			default:
				continue;
		}
	}
	//int status; volendo si può aggiungere lo stato per un resoconto più preciso
	if (  signal(SIGINT,  sighandler) == SIG_ERR ||         
          signal(SIGTERM, sighandler) == SIG_ERR ||          
          signal(SIGQUIT, sighandler) == SIG_ERR ) {        
          perror("signal");
          return EXIT_FAILURE;                                 
      } 

	for (;;) {                                          // Whenever a process falls, another rises to take his place - Quando un processo cade, un altro viene creato per prenderne il posto
		if ((wait(NULL)) != -1) {
			switch (fork())
			{
				case -1:
					perror("Error in fork");
					exit(EXIT_FAILURE);
				case 0:
					Process_Work(sock,fdlock, cfg, fdal);//, db);
				default:
					continue;
			}
		}
	}	
	return EXIT_SUCCESS;
}
