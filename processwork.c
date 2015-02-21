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
#include <string.h>   //per memset


#define MAX_THREAD_NUM 20          //Massimo numero di Thread per processo
#define MAX_ERROR_ALLOWED 5        //Masimo numero di errori ignorabili

struct thread_struct {
	int conn_sd; //socket di connessione
};

void *thread_work(void *arg) {
	(void) arg;
	return 0;
}

int Process_Work(int lsock)
{
	int i, error=0, connsd;
	socklen_t client_len;
	struct sockaddr_in clientaddr;
	pthread_t tid[MAX_THREAD_NUM];
	struct thread_struct *tss[MAX_THREAD_NUM];
	
	for(i = 0; i < MAX_THREAD_NUM; i++) {
		if (pthread_create(&tid[i],NULL,thread_work,tss[i]) < 0) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}
	
	
	if (memset((void*)&clientaddr, 0, sizeof(clientaddr)) == NULL) {
		perror("memset");
		Process_Work(lsock);
	}
		
	i=0;
	while (1==1)
	{
		//semaforo                                                      //Andrà implementato un semaforo tra i processi per evitare l'effetto "Thundering Herd"
		client_len = sizeof(clientaddr);
		if ((connsd = accept(lsock, (struct sockaddr*)&clientaddr,&client_len))<0)
		{
			perror("Error in accept");
			error+=1;
			if (error <= MAX_ERROR_ALLOWED) continue;                     //Prova ad ignorare l'errore
			 else Process_Work(lsock);                                   //Troppi fallimenti, ricomincia
		}
		
		tss[i].conn_sd = connsd;
		i++
		//Thread_work                 //Da aggiungere
		if 
	}
	i=(i+1)%MAX_THREAD_NUM;
	}
}
