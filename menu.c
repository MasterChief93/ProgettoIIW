#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "db.h"
#include "fileman.h"

//aggiunta entry nel db per immagini originali
//confronto contenuto cartella con contenuto db

extern char **environ;
//extern sqlite3 *db;

int start_menu(struct Config *cfg) {
	printf("Welcome in the Server Configuration Program:\n1. Add an entry on the database for an original image\n2. Check consistency between database and the folder of original images\n3. Start server\n");
	int num = 0;
	scanf("%d",&num);
	
	if (num == 1) {
		int fd;
		sqlite3 *dbmain;
		if (sqlite3_open("db/images.db", &dbmain)){  //Open the conection to the database - Apre la connessione al database
			perror("error in sqlite_open");
			sqlite3_close(dbmain);                    //In any case of server shutdown, close the db connection first - In ogni caso di chiusura del server, chiude anche la connessione al database 
			return EXIT_FAILURE;
		}
		printf("Insert image name (\"/image_name.ext\"): ");
		char image_name[512];
		char path[600];
		scanf("%s",image_name);
		sscanf(path,"%s%s",cfg->Orig_Path,image_name);
		if ((fd = open(path,O_RDWR)) == -1)  {
			printf("An error occurred or the file doesn't exist\n");
			fflush(stdout);
			return EXIT_FAILURE;
		}
		if (close(fd) == -1) {
			printf("An error occurred closing the file\n");
			fflush(stdout);
			return EXIT_FAILURE;
		}
		if (dbcontrol(dbmain,image_name,1) == 1) {
				printf("Image %s is already on the database\n",image_name);
				fflush(stdout);
				return EXIT_SUCCESS;
		}
		struct Record rc;
		strcpy(rc.name,image_name);
		rc.acc = 0;
		if (dbadd(dbmain,rc,1) == EXIT_FAILURE) {
			printf("An error occurred with the db\n");
			fflush(stdout);
			return EXIT_FAILURE;
		} else {
			printf("Image entry for %s successfully added\n",image_name);
			fflush(stdout);
		}
		sqlite3_close(dbmain);
		return EXIT_SUCCESS;
	}
	else if (num == 2) {
		pid_t pid;
		int pfd[2];

		if (pipe(pfd) == -1) {
			printf("Error in pipe()\n");
			fflush(stdout);
			exit(EXIT_FAILURE);
		}
		if ((pid = fork()) == -1) {
			printf("Error in fork()\n");
			fflush(stdout);
			exit(EXIT_FAILURE);
		}
		else if (pid == 0) {
			//char *cmd[] = {"/bin/ls", cfg->Orig_Path};
			if (close(pfd[0]) == -1) {
				printf("Error in close()\n");
				exit(EXIT_FAILURE);
			}
			if (dup2(pfd[1], STDOUT_FILENO) == -1) {
				perror("Error in dup2()\n");
				exit(EXIT_FAILURE);
			}
			//execve(cmd[0], cmd, environ);
			if (system("/bin/ls ./orig") == -1) {
				perror("error in system()");
				exit(EXIT_FAILURE);
			}
			//perror("Error in execve()\n");
			//fflush(stdout);
			exit(EXIT_SUCCESS);
		}

		wait(NULL);
		sqlite3 *dbmain;
		if (sqlite3_open("db/images.db", &dbmain)){  //Open the conection to the database - Apre la connessione al database
			perror("error in sqlite_open");
			sqlite3_close(dbmain);                    //In any case of server shutdown, close the db connection first - In ogni caso di chiusura del server, chiude anche la connessione al database 
			return EXIT_FAILURE;
		}

		if (close(pfd[1]) == -1) {
			printf("Error in close()\n");
			fflush(stdout);
			return EXIT_FAILURE;
		}
		char *buf;
		size_t size = 1024, tot;
		buf = malloc(size * sizeof(char));
		if (!buf) {
			printf("Error in malloc()\n");
			fflush(stdout);
			return EXIT_FAILURE;
		}
		tot = 0;
		for (;;) {
			ssize_t v = read(pfd[0], buf + tot, size - tot);
			if (v == -1) {
				printf("Error in read()\n");
				fflush(stdout);
				return EXIT_FAILURE;
			}
			if (v == 0)
				break;
			tot += v;
			if (size == tot) {
				size += size;
				buf = realloc(buf, size);
				if (!buf) {
					printf("Error in realloc()\n");
					fflush(stdout);
					return EXIT_FAILURE;
				}
			}
		}
		buf[tot] = '\0';
		char *image_name_ns;
		char image_name[512];
		char path[strlen(cfg->Orig_Path) + 512];
		int fd;
		while ((image_name_ns = strtok(buf,"\n")) != NULL) {
			sprintf(path,"%s/%s",cfg->Orig_Path,image_name_ns);
			sprintf(image_name,"/%s",image_name_ns);
			struct Record rc;
			strcpy(rc.name,image_name);
			rc.acc = 0;
			int ispresent = dbcontrol(dbmain,image_name,1); 
			if (ispresent == 0) {
				dbadd(dbmain,rc,1);
				printf("Image %s added\n",image_name_ns);
				fflush(stdout);
			// } else if (ispresent == 1) {
			// 	if ((fd = open(path,O_RDWR)) == -1) {
			// 		dbremove(dbmain,image_name,1);
			// 		printf("Image %s removed\n",image_name);
			// 		fflush(stdout);
			// 	} else close(fd);
			}
			buf = NULL;
			memset(path,0,strlen(cfg->Orig_Path) + 512);
			memset(image_name,0,512);
			memset(image_name_ns,0,strlen(image_name_ns));
		}
		dbvalidate(dbmain,cfg->Orig_Path,1);
		printf("Database consistency confirmed\n");
		fflush(stdout);
		sqlite3_close(dbmain);
		return EXIT_SUCCESS;
	} 
	else if (num == 3) {
		return 2523;
	}
	else {
		printf("Not a valid command\n\n");
		fflush(stdout);
		return EXIT_FAILURE;
	}

}