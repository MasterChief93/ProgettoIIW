#ifndef FILEMAN_H                         //Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define FILEMAN_H

struct Config{
	int Serv_Port                //Porta d'ascolto del Server
	int Max_Prole_Num;            //Massimo numero processi concorrenti (oltre al padre). Si suppone che ogni processo si divida in thread.
	int Min_Thread_Num;           //Numero di Thread nel pool iniziale di ogni processo
	int Max_Thread_Num;           //Massimo numero di Thread per processo
	int Thread_Increment;         //Quanti Thread aggiungere ogni volta che il pool risulta insufficiente
	int Max_Error_Allowed;        //Massimo numero di errori ignorabili
};

extern int Load_Config(int fdc, struct Config *cfg);
extern int Set_Config_Default(int fdc, struct Config *cfg);
#endif
