#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include "threadwork.h"    //Nostro


Thread_Work(int connsd)
{
	char buff[10];
	if (strcpy(buff, "Hello!")==NULL
	{
		perror("error in strcpy");
		exit(EXIT_FAILURE);
	}
	write(STDOUT_FILENO, buff, sizeof(buff);
	return EXIT_SUCCESS;
}
