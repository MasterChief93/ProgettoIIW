#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include "threadwork.h"    //Nostro
#include "processwork.h"

int Thread_Work(int connsd)
{
	(void) connsd;
	ssize_t writen;
	char buff[10];
	if (strcpy(buff, "Hello!\0") == NULL)
	{
		perror("error in strcpy");
		exit(EXIT_FAILURE);
	}
	writen = write(STDOUT_FILENO, buff, sizeof(buff));
	if (writen == 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
