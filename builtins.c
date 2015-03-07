#include "definitions.h"

void set(command* job)
{
    if(job->args[1] == NULL) error("ERROR invalid number of args");
    
	int length = strcspn(job->args[1], "=");
	
	if(length == 0)
		return;
	
	char* envname = malloc(length * sizeof(char));
	memcpy(envname, job->args[1], length);
	
	if(strchr(job->args[1], '=') == 0)
	{
		printf("%s = %s\n", envname, getenv(envname));
		free(envname);
		return;
	}
	
	char* envval = malloc((strlen(job->args[1])-length) * sizeof(char));
	strcpy(envval, &(job->args[1][length+1]));
	
	if(setenv(envname, envval, 1) == -1) error("error setting");
	free(envname);
	free(envval);
}

void cd(command* job)
{
    char* newDir = realpath( (job->args[1] ? job->args[1] : getenv( "HOME" )), NULL);
    
	if(chdir(newDir) == -1)
	{
		switch(errno)
		{
			case ENOENT: 
				printf("File does not exist\n");
				break;
			case EACCES:
				printf("Permission was denied\n");
				break;
			case ENAMETOOLONG:
				printf("Path is too long\n");
				break;
			default:
				error("error with cd\n");
		}
	}
    
    setenv( "PWD", newDir, 1 );
    printf( "%s\n", getenv("PWD"));
    free(newDir);
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
