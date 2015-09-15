#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "fileman.h"

void page_generator(struct Config *cfg) {
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
		if (close(pfd[0]) == -1) {
			printf("Error in close()\n");
			exit(EXIT_FAILURE);
		}
		if (dup2(pfd[1], STDOUT_FILENO) == -1) {
			perror("Error in dup2()\n");
			exit(EXIT_FAILURE);
		}
		char instruction[strlen("/bin/ls") + strlen(cfg->Orig_Path)];
		sprintf(instruction,"/bin/ls %s",cfg->Orig_Path);
		if (system(instruction) == -1) {
			perror("error in system()");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}

	wait(NULL);

	if (close(pfd[1]) == -1) {
		printf("Error in close()\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	char *buf;
	size_t size = 1024, tot;
	buf = malloc(size * sizeof(char));
	if (!buf) {
		printf("Error in malloc()\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	tot = 0;
	for (;;) {
		ssize_t v = read(pfd[0], buf + tot, size - tot);
		if (v == -1) {
			printf("Error in read()\n");
			fflush(stdout);
			exit(EXIT_FAILURE);
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
				exit(EXIT_FAILURE);
			}
		}
	}
	buf[tot] = '\0';
	int fd = 0;
	if ((fd = open("default.html", O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1) {  // Create access.log, unless it already exists - Crea log.log, a meno che già non esista
		if ((fd = open("default.html", O_TRUNC | O_WRONLY)) == -1) {                      // If access.log already exists, open it - Se log.log già esiste, aprilo
			perror("Error in opening default.html");
			return (EXIT_FAILURE);
		}
	}

	dprintf(fd,"<!DOCTYPE html>\n<html>\n<head>\n<title>Main Page</title>\n</head>\n");
	dprintf(fd,"<body>\n\n<h1>Scegli l'immagine</h1>\n");
	int count = 1;
	char *filename;
	while ((filename = strtok(buf,"\n")) != NULL) {
		dprintf(fd,"<a href=\"%s\">Immagine %d</a><br>\n",filename,count);
		count += 1;
		buf = NULL;
	}
	dprintf(fd,"</body>\n\n</html>");
	if (close(fd) == -1) {
		perror("error in closing default.html");
		exit(EXIT_FAILURE);
	}
}