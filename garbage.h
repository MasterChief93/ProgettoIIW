#include <sqlite3.h>
#include "fileman.h"
#ifndef FILEMAN_H                         //Include Guards per evitare di includere pi� volte l'header (ed avere quindi funzioni definite pi� volte)7
#define FILEMAN_H
extern int Garbage_Collector(sqlite3 *db, struct Config *cfg);      //Every "Garbage_Collection_Frequence" seconds, removes the least recently seen images, untill "Max_Cache_Size" is reached - Ogni "Garbage_Collection_Frequence" secondi rimuove le immagini non accedute da pi� tempo, fino a raggiungere "Max_Cache_Size"
#endif