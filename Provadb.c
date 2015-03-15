#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sqlite3.h>

/*
int callback (void * NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	printf ("argc = %d\n", argc);
	printf("argv0=%s\n", argv[0]);
    for(i=0; i<argc; i++){
		printf("%d\n", i);
		fflush(stdout); 
		if (strcmp(azColName[i], "one")==0 )
		{
			printf("%s = %s\n", azColName[i], argv[i]);
		}
		else printf("%s = %d\n", azColName[i], atoi(argv[i]));
    }
    printf("\n");
    return 0;
  
}


int main()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	
	errno=0;
	if (sqlite3_open("db/test.db", &db)){
		perror("error in sqlite_open");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}
	if (sqlite3_exec(db, "select count(*) from tb1", callback, 0, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	sqlite3_close(db);
	
	return EXIT_SUCCESS;
	
	
}
*/

/*
int callback (void * res, int argc, char **argv, char **azColName)
{
	int * i=(int *) res;
	long j;
	char *endptr;
	printf("argv0=%s\n", argv[0]);
	errno=0;
	j=strtol(argv[0], &endptr, 0);
	if (errno!=0)
	{
		perror("error in strtol");
		return EXIT_FAILURE;
	}
	*i=j;
	return EXIT_SUCCESS;  
}


int foo()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int *res;
	
	errno=0;
	
	res = malloc(sizeof(int));
	*res=0;
	if (sqlite3_open("db/test.db", &db)){
		perror("error in sqlite_open");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}
	if (sqlite3_exec(db, "select count(*) from tb1", callback, (void *) res, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	sqlite3_close(db);
	
	return *res;
	
	
}

int main()
{
	int result=0;
	char string[10];
	
	result =foo();
	printf("result=%d\n", result);
	snprintf(string, sizeof(char)*10, "hello");
	printf("%s\n", string);
	
	return EXIT_SUCCESS;
}
*/



/*
int main()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	char dbcomm[512], text[10];
	
	errno=0;
	if (sqlite3_open("db/test.db", &db)){
		perror("error in sqlite_open");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}
	snprintf(text, sizeof(char)*512, "%s", "image");
	printf("%s\n", text);
	snprintf(dbcomm, sizeof(char)*512, "INSERT INTO tb2 values('%s', datetime())",  text);
	printf("%s\n", dbcomm);
	errno=0;
	if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	sqlite3_close(db);
	
	return EXIT_SUCCESS;
	
	
}
* */


int callback (void * res, int argc, char **argv, char **azColName)
{
	char *val=(char *) res;
	
	printf("%s\n", argv[0]);
	strcpy(val, argv[0]);
	printf("%s\n", val);
	return EXIT_SUCCESS;  
}


int main()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	char *nameimm, dbcomm[512];
	
	nameimm= malloc(sizeof(char)*512);
	
	errno=0;
	if (sqlite3_open("db/test.db", &db)){
		perror("error in sqlite_open");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}
	errno=0;
	if (sqlite3_exec(db, "select name from tb2 where date = (select max(date) from tb2)", callback, (void *)nameimm, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	printf("%s\n", nameimm);
	snprintf(dbcomm, sizeof(char)*512, "DELETE FROM tb2 WHERE name='%s'",  nameimm);
	
	errno=0;
	if (sqlite3_exec(db, dbcomm, NULL, 0, &zErrMsg)){
		perror("error in sqlite_exec");
		sqlite3_free(zErrMsg);
		return EXIT_FAILURE;
	}
	
	
	sqlite3_close(db);
	
	return EXIT_SUCCESS;
	
	
}
