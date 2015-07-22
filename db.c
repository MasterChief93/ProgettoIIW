#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sqlite3.h>
#include "db.h"

int callbackchk (void * res, int argc, char **argv, char **azColName)
{
	long j;
	char *endptr;
	int *val = (int *) res;
	(void) azColName;
	
	if (argc!=1)
	{
		fprintf(stderr, "Unexpected number of columns");
		return EXIT_FAILURE;
	}  
	
	errno=0;
	j=strtol(argv[0], &endptr, 0);
	if (errno!=0)
	{
		perror("error in strtol");
		return EXIT_FAILURE;
	}  
	if (j==0) *val=0;
	else *val=1;
	return EXIT_SUCCESS;
}

int dbcontrol(sqlite3 *db, char *image, int flag)              //Controls whether "image" exists in table 'flag' (0=imag, 1=orig, 2=page) and returns 1 if positive, 0 if negative. - Controlla se "image" esiste nella tabella flag (0=imag, 1=orig, 2=page) e restituisce 1 in caso affermativo, 0 in caso negativo
{
	char *zErrMsg = 0;
	char *dbcomm;
	int *res;
	char flags[6];
	
	if((dbcomm = malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	
	if (flag==0) snprintf(flags, sizeof(char)*10, "imag");
	else if (flag==1) snprintf(flags, sizeof(char)*10, "orig");
	else if (flag==2) snprintf(flags, sizeof(char)*10, "page");
	else {
		fprintf(stderr, "Error in dbcontrol: wrong flag value");
		exit(EXIT_FAILURE);
	}
	
	snprintf(dbcomm, sizeof(char)*512, "SELECT count(*) FROM %s WHERE name=%s", flags, image);
	
	if (sqlite3_exec(db, dbcomm, callbackchk, (void*)res, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	return *res;
}


int dbadd(sqlite3 *db, struct Record rd, int flag)                //Adds to table 'flag' (0=imag, 1=orig, 2=page) the record rd. - Aggiunge allla tabella flag (0=imag, 1=orig, 2=page) il record rd          
{
	char *zErrMsg = 0;
	char *dbcomm;
	
	if((dbcomm = malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	
	if (flag==0) snprintf(dbcomm, sizeof(char)*512, "INSERT INTO imag values('%s', datetime(), %long",  rd.name, rd.acc);
	else if (flag==1) snprintf(dbcomm, sizeof(char)*512, "INSERT INTO orig values('%s', datetime(), %long",  rd.name, rd.acc);  //Non è normale...
	else if (flag==2) snprintf(dbcomm, sizeof(char)*512, "INSERT INTO page values('%s', datetime(), %long",  rd.name, rd.acc);
	else {
		fprintf(stderr, "Error in dbadd: wrong flag value");
		exit(EXIT_FAILURE);
	}
	
	if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
	
}

int dbremove(sqlite3 *db, char *image, int flag)                 //Removes from table 'flag' (0=imag, 1=orig, 2=page) "image". - Rimuove dallla tabella flag (0=imag, 1=orig, 2=page) "image"
{
	char *zErrMsg = 0;
	char *dbcomm;
	
	if((dbcomm = malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	
	if (flag==0) snprintf(dbcomm, sizeof(char)*512, "DELETE FROM imag WHERE name='%s'", image);
	else if (flag==1) snprintf(dbcomm, sizeof(char)*512, "DELETE FROM orig WHERE name='%s'", image);
	else if (flag==2) snprintf(dbcomm, sizeof(char)*512, "DELETE FROM page WHERE name='%s'", image);
	else {
		fprintf(stderr, "Error in dbadd: wrong flag value");
		exit(EXIT_FAILURE);
	}
	
	if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}


int callbackremol (void * res, int argc, char **argv, char **azColName)
{
	(void) azColName;
	
	if (argc!=1)
	{
		fprintf(stderr, "Unexpected number of columns");
		return EXIT_FAILURE;
	}  
	
	char *val=(char *) res;
	
	strcpy(val, argv[0]);
	return EXIT_SUCCESS;  
}


int dbremoveoldest(sqlite3 *db)                              //Removes from tables 'imag'and 'page' the least recently used record(s). -Rimuove dalle tabelle 'imag' e 'page' il/i record a cui non si è acceduto da più tempo
{
	char *zErrMsg = 0;
	char *dbcomm, *nameimm, *filepath;
	
	if ((nameimm= malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	
	if((dbcomm = malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	
	if (sqlite3_exec(db, "SELECT name FROM imag WHERE date = (SELECT min(date) FROM imag)", callbackremol, nameimm, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	dbremove(db, nameimm, 0);
	
	
	if (sqlite3_exec(db, "SELECT name FROM page WHERE date = (SELECT min(date) FROM page)", callbackremol, nameimm, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	dbremove(db, nameimm, 2);
	
	snprintf(filepath, sizeof(char)*512, "/bin/rm %s.*", nameimm);       //Removes the files from the system - Rimuove i file dal sistema
	if (system(filepath)==-1){
		perror("error in system (rm)");                                  //NOTA: Se non dovesse funzionare, probabilmente andrà aggiunto ./ a nameimm
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

int callbackacc (void * res, int argc, char **argv, char **azColName)
{
	int *val=(int *) res;
	long j;
	char *endptr;
	(void) azColName;
	
	if (argc!=1)
	{
		fprintf(stderr, "Unexpected number of columns");
		return EXIT_FAILURE;
	}  
	
	errno=0;
	j=strtol(argv[0], &endptr, 0);
	if (errno!=0)
	{
		perror("error in strtol");
		return EXIT_FAILURE;
	}
	*val=j+1;
	return EXIT_SUCCESS;  
}

int dbcheck(sqlite3 *db, char *image, char *origimag )           //Calls dbcontrol, updates the acces date/add the record on 'imag'and 'page' and updates the acces numer on 'orig'. -Chiama dbcontrol, aggiorna la data d'accesso/aggiunge il record su 'imag' e 'page' ed aggiorna il numero di accessi su 'orig'
{
	char *zErrMsg = 0;
	char *dbcomm;
	long *acc;
	int check;
	
	if((dbcomm = malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	
	if ((check = dbcontrol(db, image, 0)) ==1)
	{
		snprintf(dbcomm, sizeof(char)*512, "SELECT acc FROM imag WHERE name=%s",  image);
	if (sqlite3_exec(db, dbcomm, callbackacc, acc, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
		}
		
		snprintf(dbcomm, sizeof(char)*512, "UPDATE imag SET date =datetime(), acc = %long  WHERE name= '%s'", *acc, image);
		if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
			perror("error in sqlite_exec");
			sqlite3_free(zErrMsg);
			return EXIT_FAILURE;
		}
	}
	else if ((check = dbcontrol(db, image, 0)) ==0)
	{
		struct Record rd = {.name = image ,.acc = 0};
		dbadd(db, rd, 0);
	}
	
	if ((check = dbcontrol(db, image, 2)) ==1)
	{
		snprintf(dbcomm, sizeof(char)*512, "SELECT acc FROM page WHERE name=%s",  image);
	if (sqlite3_exec(db, dbcomm, callbackacc, acc, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
		}
		
		snprintf(dbcomm, sizeof(char)*512, "UPDATE page SET date =datetime(), acc = %long  WHERE name= '%s'", *acc, image);
		if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
			perror("error in sqlite_exec");
			sqlite3_free(zErrMsg);
			return EXIT_FAILURE;
		}
	}
	else if ((check = dbcontrol(db, image, 2)) ==0)
	{
		struct Record rd = {.name = image ,.acc = 0};
		dbadd(db, rd, 2);
	}
	
	
	snprintf(dbcomm, sizeof(char)*512, "SELECT acc FROM orig WHERE name=%s",  origimag);
	if (sqlite3_exec(db, dbcomm, callbackacc, acc, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	snprintf(dbcomm, sizeof(char)*512, "UPDATE orig SET acc = %long WHERE name= '%s'", *acc, origimag);
	if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	return check;
}


int callbackcount (void * res, int argc, char **argv, char **azColName)
{
	int *val=(int *) res;
	long j;
	char *endptr;
	(void) azColName;
	
	if (argc!=1)
	{
		fprintf(stderr, "Unexpected number of columns");
		return EXIT_FAILURE;
	}  
	
	errno=0;
	j=strtol(argv[0], &endptr, 0);
	if (errno!=0)
	{
		perror("error in strtol");
		return EXIT_FAILURE;
	}
	*val=j;
	return EXIT_SUCCESS;  
}

int dbcount(sqlite3 *db, int flag)              //Returns the number of existing records in 'flag' (0=imag, 1=orig, 2=page) table. - Restituisce il numero di record esistenti nella tabella flag (0=imag, 1=orig, 2=page)
{
	char *zErrMsg = 0;
	char *dbcomm;
	int *res;
	char flags[6];
	
	if((dbcomm = malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	
	if (flag==0) snprintf(flags, sizeof(char)*10, "imag");
	else if (flag==1) snprintf(flags, sizeof(char)*10, "orig");
	else if (flag==2) snprintf(flags, sizeof(char)*10, "page");
	else {
		fprintf(stderr, "Error in dbcontrol: wrong flag value");
		exit(EXIT_FAILURE);
	}
	
	snprintf(dbcomm, sizeof(char)*512, "SELECT count(*) FROM %s ", flags);
	
	if (sqlite3_exec(db, dbcomm, callbackcount, (void*)res, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	return *res;
}

int callbacksel (void * res, int argc, char **argv, char **azColName)
{
	(void) azColName;
	
	if (argc!=3)
	{
		fprintf(stderr, "Unexpected number of columns");
		return EXIT_FAILURE;
	}  
	
	char * s=(char *)res;
	snprintf(s, sizeof(char)*512, "%s %s %s", argv[0], argv[1], argv[2]);
	return EXIT_SUCCESS;
}

char *dbselect(sqlite3 *db, char *image, int flag)      //Returns the record of "image" from the table 'flag' (0=imag, 1=orig, 2=page) - Restituisce il record di "image" dalla tabella 'flag' (0=imag, 1=orig, 2=page)
{
	char *zErrMsg = 0;
	char *dbcomm;
	char *res;
	char flags[6];
	
	if((dbcomm = malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	
	if((res = malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc");
		return (EXIT_FAILURE);
	}
	
	if (flag==0) snprintf(flags, sizeof(char)*10, "imag");
	else if (flag==1) snprintf(flags, sizeof(char)*10, "orig");
	else if (flag==1) snprintf(flags, sizeof(char)*10, "page");
	else {
		fprintf(stderr, "Error in dbcontrol: wrong flag value");
		exit(EXIT_FAILURE);
	}
	
	snprintf(dbcomm, sizeof(char)*512, "SELECT (*) FROM %s WHERE name=%s", flags, image);
	
	if (sqlite3_exec(db, dbcomm, callbacksel, (void*)res, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	return *res;
}

