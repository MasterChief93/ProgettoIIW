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


int Garbage_Collector(sqlite3 *db, struct Config *cfg, int fdl) {     //Every "Garbage_Collection_Frequence" seconds, removes the least recently seen images, untill "Max_Cache_Size" is reached - Ogni "Garbage_Collection_Frequence" secondi rimuove le immagini non accedute da più tempo, fino a raggiungere "Max_Cache_Size"
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
	return EXIT_FAILURE;
}