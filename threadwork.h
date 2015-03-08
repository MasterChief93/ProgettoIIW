#include <sqlite3.h>
#ifndef THREADWORK_H                         //Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define THREADWORK_H
extern int Thread_Work(int connsd, int fdl, sqlite3 *db);
#endif
