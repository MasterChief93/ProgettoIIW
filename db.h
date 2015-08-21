#include <sqlite3.h>
#ifndef DB_H                         //Include Guards: to avoid multiple header inclusions (having so functions defined multiple times) -Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define DB_H

struct Record{
	char name[512];
	long acc;          //Number of acceses to the image - Numero di accessi all'immagine.
}; 

extern int dbcontrol(sqlite3 *db, char *image, int flag);        //Control whether "image" exists in table 'flag' (0=imag, 1=orig, 2=page) and return 1 if positive, 0 if negative. -Controlla se "image" esiste nella tabella flag (0=imag, 1=orig, 2=page) e restituisce 1 in caso affermativo, 0 in caso negativo
extern int dbadd(sqlite3 *db, struct Record rd, int flag);       //Add to table 'flag' (0=imag, 1=orig, 2=page) the record rd. -Aggiunge allla tabella flag (0=imag, 1=orig, 2=page) il record rd  
extern int dbremove(sqlite3 *db, char *image, int flag);         //Remove from table 'flag' (0=imag, 1=orig, 2=page) "image". -Rimuove dalla tabella flag (0=imag, 1=orig, 2=page) "image"
extern int dbremoveoldest(sqlite3 *db, int fdl);                  //Remove from tables 'imag'and 'page' the least recently used record(s). -Rimuove dalle tabelle 'imag' e 'page' il/i record a cui non si è acceduto da più tempo
extern int dbcheck(sqlite3 *db, char *image, char* origimag);    //Call dbcontrol, update the acces date/add the record on 'imag'and 'page' and update the acces numer on 'orig'. -Chiama dbcontrol, aggiorna la data d'accesso/aggiunge il record su 'imag' e 'page' ed aggiorna il numero di accessi su 'orig'
extern int dbcount(sqlite3 *db, int flag);                       //Return the number of existing record in 'flag' (0=imag, 1=orig, 2=page) table. -Restituisce il numero di record esistenti nella tabella flag (0=imag, 1=orig, 2=page)
extern char *dbselect(sqlite3 *db, char *image, int flag);      //Return the record of "image" from the table 'flag' (0=imag, 1=orig, 2=page) - Restituisce il record di "image" dalla tabella 'flag' (0=imag, 1=orig, 2=page)
extern char *dbfindUA (sqlite3 *db, char *UA);      //Return the maximum resolution supported by the User Agent - Restituisce la massima risoluzione supportata dall' User Agent.
extern int dbaddUA (sqlite3 *db, char *UA, char *res);         //Add a User Agent and its  supported maximum resolution to the database - Aggiunge un User Agent e la sua risoluzione massima supportata al database.
#endif
