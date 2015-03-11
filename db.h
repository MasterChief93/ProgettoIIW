#include <sqlite3.h>
#ifndef DB_H                         //Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define DB_H

struct Record{
	char[512] name;
	long acc;          //Numero di accessi all'immagine.
} 

static int dbcontrol(sqlite3 *db, char *image, int flag);        //Controlla se "image" esiste nella tabella flag (0=imag, 1=orig) e restituisce 1 in caso affermativo, 0 in caso negativo
static int dbadd(sqlite3 *db, struct Record rd);                 //Aggiunge allla tabella flag (0=imag, 1=orig) il record rd  
static int dbremove(sqlite3 *db, char *image, int flag);         //Rimuove dallla tabella flag (0=imag, 1=orig) "image"
static int dbremoveoldest(sqlite3 *db);                          //Rimuove dalla tabella imag il/i record a cui non si è acceduto da più tempo
static int dbcheck(sqlite3 *db, char *image, char* origimag);    //Chiama dbcontrol, aggiorna la data d'accesso/aggiunge il record su imag ed aggiorna il numero di accessi su orig
static int dbcount(sqlite3 *db, int flag);                       //Restituisce il numero di record esistenti nella tabella flag (0=imag, 1=orig)
#endif
