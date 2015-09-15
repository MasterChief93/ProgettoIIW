#include <sqlite3.h>
#include "fileman.h"
#ifndef FILEMAN_H                         //Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)
#define FILEMAN_H
extern int Garbage_Collector(struct Config *cfg, int fdl);      //Every "Garbage_Collection_Frequence" seconds, removes the least recently seen images, untill "Max_Cache_Size" is reached - Ogni "Garbage_Collection_Frequence" secondi rimuove le immagini non accedute da più tempo, fino a raggiungere "Max_Cache_Size"
#endif
