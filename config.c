#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include "fileman.h"


int Set_Config_Default(int fdc, struct Config *cfg)  //Creates the config.ini file, load the default values in both the program and the file - Crea il file config.ini, carica i valori di default sia nel programma che sul file
{
	FILE *streamc;
	struct Config Default = {
		.Serv_Port = 5042,
		.Max_Prole_Num = 10,
		.Min_Thread_Num = 10,
		.Max_Thread_Num = 50,
		.Thread_Increment = 5,
		.Max_Error_Allowed = 5};
	
	errno=0;
	if ((streamc = fdopen(fdc, "w+"))==NULL)
	{
		perror("Error in fdopen");
		return EXIT_FAILURE;
	}
	
	//Copia su cfg
	cfg->Serv_Port=Default.Serv_Port;
	cfg->Max_Prole_Num=Default.Max_Prole_Num;
	cfg->Min_Thread_Num=Default.Min_Thread_Num;
	cfg->Max_Thread_Num=Default.Max_Thread_Num;
	cfg->Thread_Increment=Default.Thread_Increment;
	cfg->Max_Error_Allowed=Default.Max_Error_Allowed;
	
	
	//Stampa su File
	
	if (fprintf(streamc, "%d Serv_Port   //Porta d'ascolto del Server\n", cfg->Serv_Port)<0)
	{
		fprintf(stderr, "Error in writing default Config on file/n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Max_Prole_Num   //Massimo numero processi concorrenti\n", cfg->Max_Prole_Num)<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Min_Thread_Num   //Numero di Thread nel pool iniziale di ogni processo\n", cfg->Min_Thread_Num)<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Max_Thread_Num   //Massimo numero di Thread per processo\n", cfg->Max_Thread_Num)<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Thread_Increment   //Quanti Thread aggiungere ogni volta che il pool risulta insufficiente\n", cfg->Thread_Increment)<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Max_Error_Allowed    //Massimo numero di errori ignorabili\n", cfg->Max_Error_Allowed )<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fclose(streamc)!=0)
	{
		perror("Error in fclose");
		return EXIT_FAILURE;
	}
	
	
	return EXIT_SUCCESS;
}


int Load_Config(int fdc, struct Config *cfg)  //Load in the program the values found in the config.ini file - Carica nel programma i valori trovati sul file config.ini
{
	char *temp;
	FILE *streamc;
	
	if ((streamc = fdopen (fdc, "w+"))==NULL)
	{
		perror("Error in fdopen");
		return EXIT_FAILURE;
	}
	
	if ((temp=malloc(sizeof(char)*1024))==NULL){
		perror("Error in malloc");
		return EXIT_FAILURE;
	}
	
	//Lettura da File
	
	if (fscanf(streamc, "%d", &(cfg->Serv_Port))!=1)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (fgets(temp, 1024*sizeof(char), streamc)==NULL)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (feof(streamc)!=0)
	{
		fprintf(stderr, "Error in reading Config from file: unexpected EOF\n");
		return EXIT_FAILURE;
	}
	
	if (fscanf(streamc, "%d", &(cfg->Max_Prole_Num))!=1)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (fgets(temp, 1024*sizeof(char), streamc)==NULL)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (feof(streamc)!=0)
	{
		fprintf(stderr, "Error in reading Config from file: unexpected EOF\n");
		return EXIT_FAILURE;
	}
	
	if (fscanf(streamc, "%d", &(cfg->Min_Thread_Num))!=1)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (fgets(temp, 1024*sizeof(char), streamc)==NULL)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (feof(streamc)!=0)
	{
		fprintf(stderr, "Error in reading Config from file: unexpected EOF\n");
		return EXIT_FAILURE;
	}
	
	if (fscanf(streamc, "%d", &(cfg->Max_Thread_Num))!=1)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (fgets(temp, 1024*sizeof(char), streamc)==NULL)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (feof(streamc)!=0)
	{
		fprintf(stderr, "Error in reading Config from file: unexpected EOF\n");
		return EXIT_FAILURE;
	}
	
	if (fscanf(streamc, "%d", &(cfg->Thread_Increment))!=1)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (fgets(temp, 1024*sizeof(char), streamc)==NULL)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	if (feof(streamc)!=0)
	{
		fprintf(stderr, "Error in reading Config from file: unexpected EOF\n");
		return EXIT_FAILURE;
	}
	
	if (fscanf(streamc, "%d", &(cfg->Max_Error_Allowed))!=1)
	{
		fprintf(stderr, "Error in reading Config from file\n");
		return EXIT_FAILURE;
	}
	
	if (fclose(streamc)!=0)
	{
		perror("Error in fclose");
		return EXIT_FAILURE;
	}
	
	
	return EXIT_SUCCESS;
}
