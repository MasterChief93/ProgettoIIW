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
#include <sqlite3.h>
#include <string.h>   	     
#include "processwork.h"     
#include "threadwork.h"
#include "fileman.h"
#include "db.h"

pthread_mutex_t mtx_struct = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_variable = PTHREAD_COND_INITIALIZER;

struct thread_struct {
	int conn_sd; 				   //Connection socket - Socket di connessione
	int count;                     //Available thread counter - Contatore dei thread disponibili
	int fdl;                       //Logging file - File di logging
	char *orig;						//String containing the relative path of the folder for the original images
	char *modif;					//String containing the relative path of the folder for the modified images
	int test_flag;					//Test flag for enabling or disabling the test mode
	int ctrl_flag;					//Thanks to this flag there will be a sort of order in the operations
};



void *thread_work(void *arg) {
	struct thread_struct *data = (struct thread_struct *) arg;
	int connsd, fdl, test;
	char *orig;
	char *modif;

	printf("Sono un thread!\n");
	fflush(stdout);
	while (1==1){
		if (pthread_mutex_lock(&mtx_cond) < 0) {
			perror("pthread_mutex_lock");
			exit(EXIT_FAILURE);  
		}
		if (pthread_cond_wait(&cond_variable,&mtx_cond) != 0) {      //The thread sleeps until (one of them, casually) receives a signal - Il thread dorme finchè (uno, a caso) non riceve un segnale.
			perror("pthread_cond_wait");
			exit(EXIT_FAILURE);
		}
		
		if (pthread_mutex_unlock(&mtx_cond) < 0) {
			perror("pthred_mutex_lock");
			exit(EXIT_FAILURE);
		}

		for (;;) {
			if (pthread_mutex_lock(&mtx_struct) < 0) {
				perror("pthread_mutex_lock");
				exit(EXIT_FAILURE);
			}
			if (data->ctrl_flag == 1) {
				connsd = data->conn_sd;
				data->count -= 1;
				fdl= data->fdl;
				orig = data->orig;
				test = data->test_flag;
				modif = data->modif;
				data->ctrl_flag = 0;
				if (pthread_mutex_unlock(&mtx_struct) < 0) {
					perror("pthread_mutex_unlock");
					exit(EXIT_FAILURE);
				}
				break;
			}
			if (pthread_mutex_unlock(&mtx_struct) < 0) {
				perror("pthread_mutex_unlock");
				exit(EXIT_FAILURE);
			}
		}
		//Once the data has been read, the Thread_Work will start
		Thread_Work(connsd, fdl, orig, modif, test);
	
		
		if (pthread_mutex_lock(&mtx_struct) < 0) {
			perror("pthred_mutex_lock");
			exit(EXIT_FAILURE);
		}
		
		data->count += 1;		//available thread counter update
		
		if (pthread_mutex_unlock(&mtx_struct) < 0) {
			perror("pthread_mutex_unlock");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

int Process_Work(int lsock, int fdlock, struct Config *cfg,  int fdl)
{
	int i, error=0, connsd, thread_num, countt;
	socklen_t client_len;
	struct sockaddr_in clientaddr;
	pthread_t tid;                          
	struct thread_struct *tss;              
	
	if((tss = malloc(sizeof(struct thread_struct))) == NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}

	

	errno=0;	

	//struct initialization
	tss->conn_sd = -1;
	tss->count = cfg->Min_Thread_Num; 								
	tss->fdl = fdl;
	tss->orig = cfg->Orig_Path;
	tss->modif = cfg->Modified_Path;
	tss->test_flag = cfg->Test_Flag;
	tss->ctrl_flag = 0;

	for(i = 0; i < cfg->Min_Thread_Num; i++) {   //Prethreading - Prethreading
		if (pthread_create(&tid,NULL,thread_work,tss) < 0) {
			perror("pthread_create");
			
			exit(EXIT_FAILURE);
		}
	}
	
	thread_num = cfg->Min_Thread_Num;
	
	if (memset((void*)&clientaddr, 0, sizeof(clientaddr)) == NULL) {
		perror("memset");
		
		exit(EXIT_FAILURE);
	}
		
	i=0;
	client_len = sizeof(clientaddr);                                         
	int value;
	while (1==1)
	{
		if (lockf(fdlock, F_LOCK,0) == -1) {
			perror("lockf");
			
			exit(EXIT_FAILURE);
		}
		if ((connsd = accept(lsock, (struct sockaddr*)&clientaddr,&client_len)) < 0)
		{
			perror("Error in accept");
			error+=1;
			if (errno != EAGAIN && error <= cfg->Max_Error_Allowed) continue;                    //Try to ignore the error - Prova ad ignorare l'errore
			 else {															  //The accept will be retried for Max_Error_Allowed number of times
			 	if (lockf(fdlock, F_ULOCK,0) == -1) {						  //If the number is reach, the process will be terminate
					perror("lockf");
					exit(EXIT_FAILURE);
				}
				perror("max error reached");
				exit(EXIT_FAILURE);  
			}                                                            //Too many failures, restart - Troppi fallimenti, ricomincia
		}
		error = 0;   												//error must be reset
		
		if (lockf(fdlock, F_ULOCK,0) == -1) {
			perror("lockf");
			exit(EXIT_FAILURE);
		}
		
		// In this cycle the parent thread will try to write on the string if it can
		for (;;) {
			if (pthread_mutex_lock(&mtx_struct) < 0) {
				perror("pthread_mutex_lock");
				
				exit(EXIT_FAILURE);
			}
			//ctrl-flag = 0 means that the parent can write, if it is 1 means that the parent have to wait for the child to read
			if (tss->ctrl_flag == 0) {
				tss->conn_sd = connsd;
				countt = (tss->count) - 1;
				tss->ctrl_flag = 1;
				if (pthread_mutex_unlock(&mtx_struct) < 0) {
					perror("pthread_mutex_unlock");
					
					exit(EXIT_FAILURE);
				}
				break;
			}
			if (pthread_mutex_unlock(&mtx_struct) < 0) {
				perror("pthread_mutex_unlock");
				
				exit(EXIT_FAILURE);
			}
		}
		
		//Now I can signal one of the threads
		if (pthread_cond_signal(&cond_variable) < 0) {
			perror("pthread_cond_signal");
			
			exit(EXIT_FAILURE);
		}

		//countt contains the number of free threads (the -1 prevents the decreasing of the counter that will happen in the thread)		
		if ((countt <= (int) (0.1*thread_num)) && (thread_num < cfg->Max_Thread_Num))       //If 90% of the threads are busy, and MAX_THREAD_NUMBER hasn't been reached, increase the pool - Se il 90% dei thread sono impegnati, e non si è arrivati a MAX_THREAD_NUM, incrementa il pool
		{
			if (pthread_mutex_lock(&mtx_struct) < 0) {
				perror("pthread_mutex_lock");
				
				exit(EXIT_FAILURE);
			}
			perror("Faccio un incremento");
			
			for(i = 0; i <= cfg->Thread_Increment; i++) {
				if (pthread_create(&tid,NULL,thread_work,tss) < 0) {
					perror("pthread_create (pool increment)");
					
					exit(EXIT_FAILURE);
				}
			}
			tss->count+=cfg->Thread_Increment;
			
			if (pthread_mutex_unlock(&mtx_struct) < 0) {
				perror("pthread_mutex_unlock");
				
				exit(EXIT_FAILURE);
			}
			thread_num+=cfg->Thread_Increment;	
		}
	}
}
