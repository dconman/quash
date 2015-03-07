#include "definitions.h"

void set(command* job)
{
	int length = strcspn(job->args, "=");
	
	if(length == 0)
		return;
	
	char* envname = malloc(length * sizeof(char));
	memcpy(envname, job->args, length);
	
	if(strchr(job->args, '=') == 0)
	{
		printf("%s = %s\n", envname, getenv(envname));
		free(envname);
		return;
	}
	
	char* envval = malloc((strlen(job->args)-length) * sizeof(char));
	strcpy(envval, &(job->args[length+1]));
	
	if(setenv(envname, envval, 1) == -1) error("error setting");
	free(envname);
	free(envval);
}

void cd(command* job)
{
	if(chdir(job->args) == -1)
	{
		switch(errno)
		{
			case ENOENT: 
				printf("File does not exist");
				break;
			case EACCES:
				printf("Permission was denied");
				break;
			case ENAMETOOLONG:
				printf("Path is too long");
				break;
			default:
				error("error with cd");
		}
	}
    
    setenv( "PWD", realpath( job->args, 0 ), 1 );
    printf( "%s\n", getenv("PWD"));
}

void quit(command* job)
{
	kill(0, SIGKILL);
	exit(0);
}

void jobs(command* job)
{
	int i;
	for(i = 0; i < max_jobs; i++)
	{
		if(background_jobs[i] == NULL)
			continue;
		printf("[%d] %d %s\n", i, background_jobs[i]->pid, background_jobs[i]->function);
	}
}

int builtin(command* job)
{
	if(strcmp(job->function, "set") == 0)
		set(job);
	else if(strcmp(job->function, "cd") == 0)
		cd(job);
	else if((strcmp(job->function, "quit") == 0) || (strcmp(job->function, "exit") == 0))
		quit(job);
	else if(strcmp(job->function, "jobs") == 0)
		jobs(job);
	else
		return 0;
	return 1;
}
