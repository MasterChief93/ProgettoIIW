#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "db.h"

int callbackchk (void *res, int argc, char **argv, char **azColName)
{
	long j;
	char *endptr;
	int *val = (int *) res;
	(void) azColName;
	
	if (argc!=1)
	{
		fprintf(stderr, "Unexpected number of columns(dbcontrol)");
		return EXIT_FAILURE;
	}  

	errno = 0;
	j = strtol(argv[0], &endptr, 0);
	if (errno!=0)
	{
		perror("error in strtol(dbcontrol)");
		return EXIT_FAILURE;
	}  

	if (j == 0) {
	 	*val = 0;
	} else { 
	 	*val = 1;
	}

	return EXIT_SUCCESS;
}

int dbcontrol(sqlite3 *db, char *image, int flag)              //Controls whether "image" exists in table 'flag' (0=imag, 1=orig) and returns 1 if positive, 0 if negative. - Controlla se "image" esiste nella tabella flag (0=imag, 1=orig) e restituisce 1 in caso affermativo, 0 in caso negativo
{
	char *zErrMsg = 0;
	char dbcomm[512];
	int res;
	char flags[6];
	errno = 0;
	


	if 		(flag==0) snprintf(flags, sizeof(char)*5, "imag");
	else if (flag==1) snprintf(flags, sizeof(char)*5, "orig");
	else {
		fprintf(stderr, "Error in dbcontrol: wrong flag value");
		exit(EXIT_FAILURE);
	}

	ssize_t cou = snprintf(dbcomm, sizeof(char)*512, "SELECT count(*) FROM %s WHERE name='%s'", flags, image);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbcontrol)");
		return EXIT_FAILURE;
	}

	if (sqlite3_exec(db, dbcomm, callbackchk, (void *)&res, &zErrMsg)) {
		perror("error in sqlite_execcontrol(dbcontrol)");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	return res;
}

struct datares {
	sqlite3 *db;
	char orig_path[512];
};


int callbackval (void *res, int argc, char **argv, char **azColName)
{
	long j;
	char *endptr;
	struct datares *val = (struct datares *) res;
	(void) azColName;
	char path[strlen(val->orig_path)+strlen(argv[0])];
	if (argc!=1)
	{
		fprintf(stderr, "Unexpected number of columns(dbvalidate)");
		return EXIT_FAILURE;
	}  
	sprintf(path,"%s%s",val->orig_path,argv[0]);
	if (open(path,O_RDWR) == -1) {
		dbremove(val->db,argv[0],1);
		printf("Image %s removed\n",argv[0]);
	}

	return EXIT_SUCCESS;
}

int dbvalidate(sqlite3 *db, char *orig_path, int flag)              //Removes from table 'flag' (0=imag, 1=orig) every image that isn't present in the directory 'orig_path' - Rimuove dalla tabella 'flag' (0=imag, 1=orig) ogni immagine che non sia presente nella cartella 'orig_path'
{
	char *zErrMsg = 0;
	char dbcomm[512];
	int res;
	char flags[6];
	errno = 0;
	struct datares datar;
	datar.db = db;
	strcpy(datar.orig_path,orig_path);


	if 		(flag==0) snprintf(flags, sizeof(char)*5, "imag");
	else if (flag==1) snprintf(flags, sizeof(char)*5, "orig");
	else {
		fprintf(stderr, "Error in dbvalidate: wrong flag value");
		exit(EXIT_FAILURE);
	}

	ssize_t cou = snprintf(dbcomm, sizeof(char)*512, "SELECT name FROM %s ", flags);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbvalidate)");
		return EXIT_FAILURE;
	}

	if (sqlite3_exec(db, dbcomm, callbackval, (void *)&datar, &zErrMsg)) {
		perror("error in sqlite_execcontrol(dbvalidate)");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	return res;
}

int dbadd(sqlite3 *db, struct Record rd, int flag)                //Adds to table 'flag' (0=imag, 1=orig) the record rd. - Aggiunge allla tabella flag (0=imag, 1=orig) il record rd          
{
	char *zErrMsg = 0;
	char dbcomm[512];
	
	
	ssize_t cou;
	if (flag==0) cou = snprintf(dbcomm, sizeof(char)*512, "INSERT INTO imag values('%s', datetime(), %ld)",  rd.name, rd.acc);
	else if (flag==1) cou = snprintf(dbcomm, sizeof(char)*512, "INSERT INTO orig values('%s', datetime(), %ld)",  rd.name, rd.acc);  
	else {
		fprintf(stderr, "Error in dbadd: wrong flag value");
		return (EXIT_FAILURE);
	}
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbadd)");
		return EXIT_FAILURE;
	}
	/*
	printf("%s\n",dbcomm);
	fflush(stdout);
	 */
	errno = 0;
	if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
		perror("error in sqlite_execcontrol (dbadd)");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
	
}

int dbremove(sqlite3 *db, char *image, int flag)                 //Removes from table 'flag' (0=imag, 1=orig) "image". - Rimuove dallla tabella flag (0=imag, 1=orig) "image"
{
	char *zErrMsg = 0;
	char dbcomm[512];
	

	ssize_t cou;
	if (flag==0) cou = snprintf(dbcomm, sizeof(char)*512, "DELETE FROM imag WHERE name='%s'", image);
	else if (flag==1) cou = snprintf(dbcomm, sizeof(char)*512, "DELETE FROM orig WHERE name='%s'", image);
	else {
		fprintf(stderr, "Error in dbremove: wrong flag value");
		exit(EXIT_FAILURE);
	}
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbremove)");
		return EXIT_FAILURE;
	}
	if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
		perror("error in sqlite_exec (dbremove));
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
		fprintf(stderr, "Unexpected number of columns (dbremoveoldest)");
		return EXIT_FAILURE;
	}  
	
	char *val=(char *) res;
	
	strcpy(val, argv[0]);
	return EXIT_SUCCESS;  
}


int dbremoveoldest(sqlite3 *db, int fdl)                              //Removes from table 'imag' the least recently used record(s). -Rimuove dalla tabella 'imag' il/i record a cui non si è acceduto da più tempo
{
	char *zErrMsg = 0;
	char nameimm[512];
	char filepath[512];
	

	
	if (sqlite3_exec(db, "SELECT name FROM imag WHERE date = (SELECT min(date) FROM imag)", callbackremol, nameimm, &zErrMsg)){
		perror("error in sqlite_exec (dbremoveoldest)");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	dbremove(db, nameimm, 0);
	
	
	dprintf("%s image correctly removed from db",nameimm);

	ssize_t cou = snprintf(filepath, sizeof(char)*512, "/bin/rm %s.*", nameimm);       //Removes the files from the system - Rimuove i file dal sistema
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbremoveoldest)");
		return EXIT_FAILURE;
	}
	if (system(filepath)==-1){
		perror("error in system (rm) (dbremoveoldest)");                                  //NOTA: Se non dovesse funzionare, probabilmente andrà aggiunto ./ a nameimm
		return EXIT_FAILURE;
	}
	dprintf("%s image correctly removed from the disk",nameimm);

	return EXIT_SUCCESS;
}

int callbackacc (void * res, int argc, char **argv, char **azColName)
{
	long *val=(long *) res;
	long j;
	char *endptr;
	(void) azColName;
	
	if (argc!=1)
	{
		fprintf(stderr, "Unexpected number of columns (dbcheck)");
		return EXIT_FAILURE;
	}  
	
	errno=0;
	j=strtol(argv[0], &endptr, 0);
	if (errno!=0)
	{
		perror("error in strtol (dbcheck)");
		return EXIT_FAILURE;
	}
	*val=j+1;
	return EXIT_SUCCESS;  
}

int dbcheck(sqlite3 *db, char *image, char *origimag )           //Calls dbcontrol, updates the acces date/add the record on 'imag' and updates the acces numer on 'orig'. -Chiama dbcontrol, aggiorna la data d'accesso/aggiunge il record su 'imag' ed aggiorna il numero di accessi su 'orig'
{
	char *zErrMsg = 0;
	char dbcomm[512];
	long acc;
	int check;
	ssize_t cou;
	errno = 0;
	
	check = dbcontrol(db, image, 0);
	if (check == 1)
	{
		cou = snprintf(dbcomm, sizeof(char)*512, "SELECT acc FROM imag WHERE name = '%s'",  image);
		if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbcheck)");
		return EXIT_FAILURE;
	}
	if (sqlite3_exec(db, dbcomm, callbackacc, (void *)&acc, &zErrMsg)){
		perror("error in sqlite_exec (dbcheck)");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
		}
		
		cou = snprintf(dbcomm, sizeof(char)*512, "UPDATE imag SET date = datetime(), acc = %ld  WHERE name= '%s'", acc, image);
		if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbcheck)");
		return EXIT_FAILURE;
	}
		if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
			perror("error in sqlite_exec (dbcheck)");
			sqlite3_free(zErrMsg);
			return EXIT_FAILURE;
		}
	}
	else if (check ==0)
	{
		struct Record rd = {.name = image ,.acc = 0};
		strcpy(rd.name,image);
		dbadd(db, rd, 0);
	}
	
	
	cou = snprintf(dbcomm, sizeof(char)*512, "SELECT acc FROM orig WHERE name = '%s'",  origimag);
	if (sqlite3_exec(db, dbcomm, callbackacc, (void *)&acc, &zErrMsg)){
		perror("error in sqlite_exec (dbcheck)");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbcheck)");
		return EXIT_FAILURE;
	}
	cou = snprintf(dbcomm, sizeof(char)*512, "UPDATE orig SET acc = %ld WHERE name= '%s'", acc, origimag);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbcheck)");
		return EXIT_FAILURE;
	}
	if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
		perror("error in sqlite_exec (dbcheck)");
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
		fprintf(stderr, "Unexpected number of columns (dbcount)");
		return EXIT_FAILURE;
	}  
	
	errno=0;
	j=strtol(argv[0], &endptr, 0);
	if (errno!=0)
	{
		perror("error in strtol (dbcount)");
		return EXIT_FAILURE;
	}
	*val=j;
	return EXIT_SUCCESS;  
}

int dbcount(sqlite3 *db, int flag)              //Returns the number of existing records in 'flag' (0=imag, 1=orig) table. - Restituisce il numero di record esistenti nella tabella flag (0=imag, 1=orig)
{
	char *zErrMsg = 0;
	char dbcomm[512];
	int res;
	char flags[6];
	errno = 0;
	
	ssize_t cou;
	if (flag==0) cou = snprintf(flags, sizeof(char)*5, "imag");
	else if (flag==1) cou = snprintf(flags, sizeof(char)*5, "orig");
	else {
		fprintf(stderr, "Error in dbcount: wrong flag value");
		exit(EXIT_FAILURE);
	}
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbcount)");
		return EXIT_FAILURE;
	}
	cou = snprintf(dbcomm, sizeof(char)*512, "SELECT count(*) FROM %s ", flags);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbcount)");
		return EXIT_FAILURE;
	}
	
	if (sqlite3_exec(db, dbcomm, callbackcount, (void*)&res, &zErrMsg)){
		perror("error in sqlite_exec (dbcount)");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	return res;
}

int callbacksel (void * res, int argc, char **argv, char **azColName)
{
	(void) azColName;
	
	if (argc!=3)
	{
		fprintf(stderr, "Unexpected number of columns (dbselect)");
		return EXIT_FAILURE;
	}  
	
	char * s=(char *)res;
	ssize_t cou = snprintf(s, sizeof(char)*512, "%s %s %s", argv[0], argv[1], argv[2]);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbselect)");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

char *dbselect(sqlite3 *db, char *image, int flag)      //Returns the record of "image" from the table 'flag' (0=imag, 1=orig) - Restituisce il record di "image" dalla tabella 'flag' (0=imag, 1=orig)
{
	char *zErrMsg = 0;
	char dbcomm[512];
	char *res;
	char flags[6];
	
	if((res= malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc (dbselect)");
		return NULL;
	}
	
	ssize_t cou;
	if (flag==0) cou = snprintf(flags, sizeof(char)*5, "imag");
	else if (flag==1) cou = snprintf(flags, sizeof(char)*5, "orig");
	else {
		fprintf(stderr, "Error in dbselect: wrong flag value");
		exit(EXIT_FAILURE);
	}
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbselect)");
		return NULL;
	}
	
	cou = snprintf(dbcomm, sizeof(char)*512, "SELECT (*) FROM %s WHERE name='%s'", flags, image);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbselect)");
		return NULL;
	}
	if (sqlite3_exec(db, dbcomm, callbacksel, (void*)res, &zErrMsg)){
		perror("error in sqlite_exec (dbselect)");
		sqlite3_free(zErrMsg);
		return NULL;
	}
	return res;
}

int callbackcount2 (void * res, int argc, char **argv, char **azColName)
{
	int *val=(int *) res;
	long j;
	char *endptr;
	(void) azColName;
	
	if (argc!=1)
	{
		fprintf(stderr, "Unexpected number of columns (dbcount2)");
		return EXIT_FAILURE;
	}  
	
	errno=0;
	j=strtol(argv[0], &endptr, 0);
	if (errno!=0)
	{
		perror("error in strtol (dbcount2)");
		return EXIT_FAILURE;
	}
	*val=j;
	return EXIT_SUCCESS;  
}

int dbcount2(sqlite3 *db, char *UA)              //Returns the number of existing records in user_agent matching UA. - Restituisce il numero di record esistenti in user agent che corrispondono a UA
{
	char *zErrMsg = 0;
	char dbcomm[512];
	int res;
	
	
		
	ssize_t cou = snprintf(dbcomm, sizeof(char)*512, "SELECT count(*) FROM user_agent WHERE UA = '%s' ", UA);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbcount2)");
		return EXIT_FAILURE;
	}
	if (sqlite3_exec(db, dbcomm, callbackcount2, (void*)&res, &zErrMsg)){
		perror("error in sqlite_exec (dbcount2)");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	return res;
}

int callbackfUA (void * res, int argc, char **argv, char **azColName)
{
	(void) azColName;
	
	
	if (argc!=1)
	{
		fprintf(stderr, "Unexpected number of columns (dbfindUA)");
		return EXIT_FAILURE;
	} 
	
	char * s=(char *)res;
	ssize_t cou = snprintf(s, sizeof(char)*512, "%s",  argv[0]);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbfindUA)");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

char *dbfindUA (sqlite3 *db, char *UA)      //Return the maximum resolution supported by the User Agent - Restituisce la massima risoluzione supportata dall' User Agent.
{
	char *zErrMsg = 0;
	char dbcomm[512];
	char *res;
	errno = 0;

	if (dbcount2(db, UA) == 0)           //If the User Agent isn't found in the db, return NULL - Se l'User Agent non è trovato nel db, ritorna NULL
	{
		return "NULL";
	}

	if((res= malloc(sizeof(char)*512))==NULL)
	{
		perror ("Error in Malloc (dbfindUA)");
		return "NULL";
	}
	
	ssize_t cou = snprintf(dbcomm, sizeof(char)*512, "SELECT resolution FROM user_agent WHERE UA = '%s'",  UA);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbfindUA)");
		return "NULL";
	}
	
	if (sqlite3_exec(db, dbcomm, callbackfUA, (void*)res, &zErrMsg)){
		perror("error in sqlite_exec (dbfindUA)");
		sqlite3_free(zErrMsg);
		return "NULL";
	}
	return res;
}

int dbaddUA (sqlite3 *db, char *UA, char *res)         //Add a User Agent and its  supported maximum resolution to the database - Aggiunge un User Agent e la sua risoluzione massima supportata al database.
{
	char *zErrMsg = 0;
	char dbcomm[512];
	errno = 0;
	
	
	ssize_t cou = snprintf(dbcomm, sizeof(char)*512, "INSERT INTO user_agent values('%s',  '%s')",  UA, res);
	if (cou > sizeof(char)*512 || cou == -1) {
		perror("snprintf (dbaddUA)");
		return EXIT_FAILURE;
	}
	/*
	printf("%s\n",dbcomm);
	fflush(stdout);
	 */

	errno = 0;
	if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
		perror("error in sqlite_execcontrol (dbaddUA)");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

