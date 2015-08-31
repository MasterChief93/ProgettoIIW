#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sqlite3.h>

#include "fileman.h"
#include "db.h"
#include "garbage.h"


int Garbage_Collector(/*sqlite3 *db,*/ struct Config *cfg, int fdl) {     //Every "Garbage_Collection_Frequence" seconds, removes the least recently seen images, untill "Max_Cache_Size" is reached - Ogni "Garbage_Collection_Frequence" secondi rimuove le immagini non accedute da più tempo, fino a raggiungere "Max_Cache_Size"
	sqlite3 *db;
	if (sqlite3_open("db/images.db", &db)){  //Open the conection to the database - Apre la connessione al database
		perror("error in sqlite_open");
		sqlite3_close(db);                    //In any case of server shutdown, close the db connection first - In ogni caso di chiusura del server, chiude anche la connessione al database 
		return EXIT_FAILURE;
	}
	while (1==1){
		sleep(cfg->Garbage_Collection_Frequence);
		sqlite3_mutex_enter(sqlite3_db_mutex(db));
		while (dbcount(db, 0) > cfg->Max_Cache_Size){
			dprintf(fdl,"%d image found: clean operation required\n");
			sqlite3_mutex_enter(sqlite3_db_mutex(db));
			if (dbremoveoldest(db,fdl) == EXIT_FAILURE){
				perror("Error in dbremoveoldest");
				break;
			}
			sqlite3_mutex_leave(sqlite3_db_mutex(db));
		}
		sqlite3_mutex_leave(sqlite3_db_mutex(db));
	}
	sqlite3_close(db);
	return EXIT_FAILURE;
}