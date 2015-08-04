#include <sqlite3.h>
#ifndef THREADWORK_H                         //Include Guards: to avoid multiple header inclusions (having so functions defined multiple times) -Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define THREADWORK_H
extern int Thread_Work(int connsd, int fdl, sqlite3 *db, char *orig, char *modif);
#endif
