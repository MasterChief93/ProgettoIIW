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
#include "processwork.h"     //Nostro
#include "threadwork.h"
#include "fileman.h"

/*
#define SERV_PORT 5042
#define MAX_PROLE_NUM 10    //Massimo numero processi concorrenti (oltre al padre). Si suppone che ogni processo si divida in thread.
*/


int main()
{
	int fde,fdc, sock, i, mem;
	struct sockaddr_in servaddr;
	//pid_t pid[MAX_PROLE_NUM];
	sem_t *semaphore;
	struct Config *cfg;
	

	if ((fde = open("error.log", O_CREAT | O_WRONLY, 0666)) == -1) {
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
	
	/*
	if ((cfg=malloc(sizeof(struct Config)))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	*/
	
	if ((mem=shmget(IPC_PRIVATE, sizeof(struct Config), O_CREAT|0666))==-1)
	{
		perror("Error in shmget");
		return (EXIT_FAILURE);
	}
	
	if ((cfg=shmat(mem, NULL, 0))==NULL)
	{
		perror("Error in shmat");
		return (EXIT_FAILURE);
	}
	
	
	if ((fdc = open("config.ini", O_CREAT | O_EXCL| O_RDWR, 0666)) == -1) {  //Crea config.ini, a meno che già non esista
		if ((fdc = open("config.ini", O_RDWR)) == -1) {                      // Se config.ini già esiste, aprilo
			perror("Error in opening config.ini");
			return (EXIT_FAILURE);
		}
		if (Load_Config(fdc, cfg)==EXIT_FAILURE)                             //Carica i valori presenti sul file config.ini
		{
			perror("Error in loading from config.ini");
			return (EXIT_FAILURE);
		}                                                  
	}
	
	else{
		if (Set_Config_Default(fdc, cfg)==EXIT_FAILURE)                      //Se non esisteva, inizializza le voci ai valori di default
		{
			perror("Error in creating config.ini . Try deleting it manually.");
			return (EXIT_FAILURE);
		}                                          
	}
	
	//Create_log_file;

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

	if(bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
	{
		perror("Error in bind");
		return (EXIT_FAILURE);
	}
	
	if (listen(sock, SOMAXCONN)<0)   //SOMAXCONN = acccetta connessioni finchè il SO non getta la spugna
	{
		perror("Error in listen");
		return (EXIT_FAILURE);
	}
	
	if ((mem=shmget(IPC_PRIVATE, sizeof(sem_t), O_CREAT|0666))==-1)
	{
		perror("Error in shmget");
		return (EXIT_FAILURE);
	}
	
	if ((semaphore=shmat(mem, NULL, 0))==NULL)
	{
		perror("Error in shmat");
		return (EXIT_FAILURE);
	}
	
	if (sem_init(semaphore, 1, 1) == -1) {
			perror("sem_init");
			return EXIT_FAILURE;
		}
	
	for (i=1; i <= cfg->Max_Prole_Num; i++)
	{
		switch (fork())    //Funziona?
		{
			case -1:
				perror("Error in prefork");
				exit(EXIT_FAILURE);
			case 0:
				printf("Sono un figlio\n");
				fflush(stdout);
				Process_Work(sock, semaphore, cfg);  //Da aggiungere in un nostro header
			default:
				continue;
		}
	}
	//int status; volendo si può aggiungere lo stato per un reseconto più preciso
	
	for (;;) {
		if ((wait(NULL)) != -1) {
			switch (fork())
			{
				case -1:
					perror("Error in fork");
					exit(EXIT_FAILURE);
				case 0:
					Process_Work(sock,semaphore, cfg);
				default:
					continue;
			}
		}
	}
			


	return 0;
}
