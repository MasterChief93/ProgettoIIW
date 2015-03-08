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


int callback (void * res, int argc, char **argv, char **azColName)
{
	int * i=(int *) res;
	int j;
	char *endptr;
	printf("argv0=%s\n", argv[0]);
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
	
	result =foo();
	printf("result=%d\n", result);
	
	return EXIT_SUCCESS;
}

