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
#include <semaphore.h>
#include <string.h>   	     //per memset
#include "processwork.h"     //Nostro
#include "threadwork.h"

#define MIN_THREAD_NUM 10          //Numero di Thread nel pool iniziale di ogni processo
#define MAX_THREAD_NUM 50          //Massimo numero di Thread per processo
#define MAX_ERROR_ALLOWED 5        //Masimo numero di errori ignorabili
#define THREAD_INCREMENT 5         //Quanti Thread aggiungere ogni volta che il pool risulta insufficiente

pthread_mutex_t mtx_struct = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_variable = PTHREAD_COND_INITIALIZER;

struct thread_struct {
	int conn_sd; 				   //socket di connessione
	int count;                     //contatore dei thread disponibili
};

void *thread_work(void *arg) {
	struct thread_struct *data = (struct thread_struct *) arg;
	int connsd;
	printf("Sono un thread!\n");
	fflush(stdout);
	while (1==1){
		if (pthread_mutex_lock(&mtx_cond) < 0) {
			perror("pthread_mutex_lock");
			exit(EXIT_FAILURE);  //dovrebbero essere pthread_exit
		}
		if (pthread_cond_wait(&cond_variable,&mtx_cond) < 0) {
			perror("pthread_cond_wait");
			exit(EXIT_FAILURE);
		}
		if (pthread_mutex_unlock(&mtx_cond) < 0) {
			perror("pthred_mutex_lock");
			exit(EXIT_FAILURE);
		}
		if (pthread_mutex_lock(&mtx_struct) < 0) {
			perror("pthred_mutex_lock");
			exit(EXIT_FAILURE);
		}
		connsd = data->conn_sd;
		data->count -= 1;
		if (pthread_mutex_unlock(&mtx_struct) < 0) {
			perror("pthread_mutex_unlock");
			exit(EXIT_FAILURE);
		}
		Thread_Work(connsd);
		//Real_Work();             //Leggere richiesta e far partire funzione adatta (unica funzione nel nostro caso), presente su altro file (per modularità)
		//Aggiornare Log
		if (pthread_mutex_lock(&mtx_struct) < 0) {
			perror("pthred_mutex_lock");
			exit(EXIT_FAILURE);
		}
		data->count += 1;
		if (pthread_mutex_unlock(&mtx_struct) < 0) {
			perror("pthread_mutex_unlock");
			exit(EXIT_FAILURE);
		}
		}
	return 0;
}

int Process_Work(int lsock, sem_t *sem)
{
	int i, error=0, connsd, thread_num, countt ;//, round=0;
	socklen_t client_len;
	struct sockaddr_in clientaddr;
	pthread_t tid;                           //[MIN_THREAD_NUM];
	
	struct thread_struct *tss;               //[MIN_THREAD_NUM];
	
	tss = malloc(sizeof(struct thread_struct));

	errno=0;	
	if (tss == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	
	tss->conn_sd = -1;
	tss->count = MIN_THREAD_NUM; 								//prima o dopo?

	for(i = 0; i < MIN_THREAD_NUM; i++) {
		if (pthread_create(&tid,NULL,thread_work,tss) < 0) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}
	
	thread_num = MIN_THREAD_NUM;
	
	if (memset((void*)&clientaddr, 0, sizeof(clientaddr)) == NULL) {
		perror("memset");
		exit(EXIT_FAILURE);
	}
		
	i=0;
	client_len = sizeof(clientaddr);                                         //Esiste la possibilità (remota) che vada all'interno del while
	while (1==1)
	{
		//while (1==1)
		//{
		if (sem_wait(sem) == -1) {
			perror("sem_wait");
			exit(EXIT_FAILURE);                                          //O permettiamo un certo numero di errori 
		}                                                                //Andrà implementato un semaforo tra i processi per evitare l'effetto "Thundering Herd"
		if ((connsd = accept(lsock, (struct sockaddr*)&clientaddr,&client_len)) < 0)
		{
			perror("Error in accept");
			error+=1;
			if (error <= MAX_ERROR_ALLOWED) continue;                    //Prova ad ignorare l'errore
			 else {
				 if (sem_post(sem) == -1) {
					perror("sem_post");                                  //Verficare se la chiusura improvvisa di un processo in questo punto non rende il semaforo inutilizzabile (posto a 0 con nessuno che possa incrementarlo)
					exit(EXIT_FAILURE);
				}
				 exit (EXIT_FAILURE);  
			}                                                            //Troppi fallimenti, ricomincia
		}
		if (sem_post(sem) == -1) {
			perror("sem_post");                                          //Verficare se la chiusura improvvisa di un processo in questo punto non rende il semaforo inutilizzabile (posto a 0 con nessuno che possa incrementarlo)
			exit(EXIT_FAILURE);
		}
		if (pthread_mutex_lock(&mtx_struct) < 0) {
			perror("pthread_mutex_lock");
			exit(EXIT_FAILURE);
		}
		tss->conn_sd = connsd;
		countt=(tss->count)+1;
		if (pthread_mutex_unlock(&mtx_struct) < 0) {
			perror("pthread_mutex_unlock");
			exit(EXIT_FAILURE);
		}
		
		if (pthread_cond_signal(&cond_variable) < 0) {
			perror("pthread_cond_signal");
			exit(EXIT_FAILURE);
		}
		
		if ((countt<= (int) (0.1*thread_num)) && (thread_num<MAX_THREAD_NUM))       //Se il 90% dei thread sono impegnati, e non si è arrivati a MAX_THREAD_NUM, incrementa il pool
		{
			if (pthread_mutex_lock(&mtx_struct) < 0) {
				perror("pthread_mutex_lock");
				exit(EXIT_FAILURE);
			}
			
			for(i = 0; i <= THREAD_INCREMENT; i++) {
				if (pthread_create(&tid,NULL,thread_work,tss) < 0) {
					perror("pthread_create (pool increment)");
					exit(EXIT_FAILURE);
				}
			}
			tss->count+=THREAD_INCREMENT;
			
			if (pthread_mutex_unlock(&mtx_struct) < 0) {
				perror("pthread_mutex_unlock");
				exit(EXIT_FAILURE);
			}
			thread_num+=THREAD_INCREMENT;
			
		}
		
	}
}
		//}
		/*while (1=1)
	
			//Prendi Semaforo_Thread[i]
			if (tss[i]->conn_sd==-1){
				tss[i]->conn_sd = connsd;
				round=0
			}
			//Rilascia Semaforo_Thread[i]
			else round++
			if (round=(thread_num-(int)(0.1*thread_num)))                    //Se il 90% dei thread sono impegnati, incrementa il pool
			{
				//Add Thread(THREAD_INCREASE)                                //Realloc per tid e thread_struct. Creare thread. Aggiornare thread_num
			}
			
			i=(i+1)%thread_num;
		}*/
