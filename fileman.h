#ifndef FILEMAN_H                         //Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define FILEMAN_H

struct Config{
	int Serv_Port;                    //Server listening port - Porta d'ascolto del Server
	int Max_Prole_Num;                //Maximum number of concurrent processes (excluding the original). It is assumed that every process creates several threads. - Massimo numero processi concorrenti (oltre al padre). Si suppone che ogni processo si divida in thread.
	int Min_Thread_Num;               //Number of threads in the initial pool of every process- Numero di Thread nel pool iniziale di ogni processo
	int Max_Thread_Num;               //Maximum number of thread per process - Massimo numero di Thread per processo
	int Thread_Increment;             //How many threads to add every time the pool is found lacking - Quanti Thread aggiungere ogni volta che il pool risulta insufficiente
	int Max_Error_Allowed;            //Maximum number of ignorable errors - Massimo numero di errori ignorabili
	int Max_Cache_Size                //Maximum number of files that can be kept in the cache - Massimo numero di file che possono essere tenuti in cache
	int Garbage_Collection_Frequence  //Time, in seconds, between two sweeps of the garbage collector (cache) - Tempo, in secondi, tra due passate del Garbage Collector (cache)
};

extern int Load_Config(int fdc, struct Config *cfg);          //Load in the program the values found in the config.ini file - Carica nel programma i valori trovati sul file config.ini
extern int Set_Config_Default(int fdc, struct Config *cfg);   //Creates the config.ini file, loads the default values in both the program and the file - Crea il file config.ini, carica i valori di default sia nel programma che sul file
#endif
