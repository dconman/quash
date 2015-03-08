/*********************************************
 * builtins.c
 * 
 * Handles execution of the builtin commands
 * 
 *********************************************/

#include "definitions.h"

 /*********************************************
 * set( command* job )
 * 
 * Print or set environment variable
 * 
 *********************************************/
void set(command* job)
{
    // Check there is an assignment.
    if(job->args[1] == NULL) return;
    
	int length = strcspn(job->args[1], "="); // Length of variable
	
	if(length == 0) //if there is no variable return
		return;
	
    // The name of the variable
	char* envname = malloc(length * sizeof(char)); 
	memcpy(envname, job->args[1], length);
	
	if(strchr(job->args[1], '=') == 0) //if there is no assignment
	{
		printf("%s = %s\n", envname, getenv(envname));
		free(envname);
		return;
	}
	
    // The new value
	char* envval = malloc((strlen(job->args[1])-length) * sizeof(char));
	strcpy(envval, &(job->args[1][length+1]));
	
    // Attempt to set the value
	if(setenv(envname, envval, 1) == -1) error("error setting env");
	free(envname);
	free(envval);
}

 /*********************************************
 * cd( command* job )
 * 
 * changes directory to the input, relative or absolute
 * If no input, change to HOME
 * 
 *********************************************/
void cd(command* job)
{
    char* newDir // Directory to change to
        = realpath( (job->args[1] ? job->args[1] : getenv( "HOME" )), NULL);
    
	if(chdir(newDir) == -1) // If it errors
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
                printf("CD failure\n");
				return;
		}
	}
    
    setenv( "PWD", newDir, 1 ); //Change PWD to current directory
    printf( "%s\n", getenv("PWD")); //Print current directory
    free(newDir);
}
 /*********************************************
 * quit( command* job )
 * 
 * Stops all processes and exits
 * 
 *********************************************/
void quit(command* job)
{
	kill(0, SIGKILL);
	exit(0);
}

 /*********************************************
 * jobs( command* job )
 * 
 * Prints current background jobs
 * 
 *********************************************/
void jobs(command* job)
{
	int i;
	for(i = 0; i < max_jobs; i++) //Iterate through background jobs
        if(background_jobs[i] != NULL) // Ignore null jobs
			printf("[%d] %d %s\n", i, background_jobs[i]->pid, background_jobs[i]->function);
}

 /*********************************************
 * builtin( command* job )
 * 
 * determines which builtin to run and calls
 * it. Returns 0 if job is not a builtin
 * 
 *********************************************/
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
