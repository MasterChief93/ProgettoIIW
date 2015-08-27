#include <semaphore.h>
#include <sqlite3.h>
#include "fileman.h"
#ifndef PROCESSWORK_H                         //Include Guards: to avoid multiple header inclusions (having so functions defined multiple times) -  Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define PROCESSWORK_H
extern int Process_Work(int lsock, int fdlock, struct Config *cfg, int fdl);//, sqlite3 *db);
#endif
