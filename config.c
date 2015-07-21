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
		.Max_Error_Allowed = 5,
		.Max_Cache_Size = 20,
		.Garbage_Collection_Frequence = 120};
		
	
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
	cfg->Max_Cache_Size=Default.Max_Cache_Size;
	cfg->Garbage_Collection_Frequence=Default.Garbage_Collection_Frequence;
	
	
	//Stampa su File
	
	if (fprintf(streamc, "%d Serv_Port   //Server listening port\n", cfg->Serv_Port)<0)
	{
		fprintf(stderr, "Error in writing default Config on file/n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Max_Prole_Num   //Maximum number of concurrent processes\n", cfg->Max_Prole_Num)<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Min_Thread_Num   //Number of threads in the initial pool of every process\n", cfg->Min_Thread_Num)<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Max_Thread_Num   //Maximum number of thread per process\n", cfg->Max_Thread_Num)<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Thread_Increment   //How many threads to add every time the pool is found lacking\n", cfg->Thread_Increment)<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Max_Error_Allowed    //Maximum number of ignorable errors\n", cfg->Max_Error_Allowed )<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Max_Cache_Size    //Maximum number of files that can be kept in the cache\n", cfg->Max_Cache_Size )<0)
	{
		fprintf(stderr, "Error in writing default Config on file\n");
		return EXIT_FAILURE;
	}
	
	if (fprintf(streamc, "%d Garbage_Collection_Frequence    //Time, in seconds, between two sweeps of the garbage collector\n", cfg->Garbage_Collection_Frequence )<0)
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
	
	if (fscanf(streamc, "%d", &(cfg->Max_Cache_Size))!=1)
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
	
	if (fscanf(streamc, "%d", &(cfg->Garbage_Collection_Frequence))!=1)
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
