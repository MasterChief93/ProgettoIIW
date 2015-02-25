#include <semaphore.h>
#ifndef PROCESSWORK_H                         //Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define PROCESSWORK_H
extern int Process_Work(int lsock, sem_t *sem);
#endif
