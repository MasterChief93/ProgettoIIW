#include <sqlite3.h>
#ifndef DB_H                         //Include Guards: to avoid multiple header inclusions (having so functions defined multiple times) -Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define DB_H

struct Record{
	char name[512];
	long acc;          //Number of acceses to the image - Numero di accessi all'immagine.
}; 

extern int dbcontrol(sqlite3 *db, char *image, int flag);        //Control whether "image" exists in table 'flag' (0=imag, 1=orig) and return 1 if positive, 0 if negative. -Controlla se "image" esiste nella tabella flag (0=imag, 1=orig) e restituisce 1 in caso affermativo, 0 in caso negativo
extern int dbadd(sqlite3 *db, struct Record rd, int flag);                 //Add to table 'flag' (0=imag, 1=orig) the record rd. -Aggiunge allla tabella flag (0=imag, 1=orig) il record rd  
extern int dbremove(sqlite3 *db, char *image, int flag);         //Remove from table 'flag' (0=imag, 1=orig) "image". -Rimuove dallla tabella flag (0=imag, 1=orig) "image"
extern int dbremoveoldest(sqlite3 *db);                          //Remove from table 'imag' the least recently used record(s). -Rimuove dalla tabella imag il/i record a cui non si è acceduto da più tempo
extern int dbcheck(sqlite3 *db, char *image, char* origimag);    //Call dbcontrol, update the acces date/add the record on 'imag' and update the acces numer on 'orig'. -Chiama dbcontrol, aggiorna la data d'accesso/aggiunge il record su imag ed aggiorna il numero di accessi su orig
extern int dbcount(sqlite3 *db, int flag);                       //Return the number of existing record in 'flag' (0=imag, 1=orig) table. -Restituisce il numero di record esistenti nella tabella flag (0=imag, 1=orig)
extern char *dbselect(sqlite3 *db, char *image, int flag);      //Return the record of "image" from the table 'flag' (0=imag, 1=orig) - Restituisce il record di "image" dalla tabella 'flag' (0=imag, 1=orig)
#endif
