#include "definitions.h"

void execute_function( command* job )
{
    int length = strlen( job->function );
	char* pwd = getenv("PWD");
    char* file = malloc( length*sizeof(char) );
    strcpy( file, job->function );

    dup2(job->out_src, STDOUT_FILENO);
    dup2(job->in_src,  STDIN_FILENO);
    if( !builtin( job ) )
        while( execl( file, job->function, job->args, (char *) NULL ) == -1)
        {
            if( errno != 2 ) error( "Error Executing" );
            
            //Here we check the path
            printf("Implement Path Checking!\n");
            
        }
            
    job->done = 1;
}

void execute( command* job, int background )
{
    int pid = 0;
    command* nextJob;
    
    if( background )
    {
        if( (pid = fork()) == -1)
            error( "Error Forking" );
        if(pid != 0)
        {
            job->pid = pid;
            addjob( job );
            job = job->next;
        }
    }
    
    if( pid == 0)
    {
        nextJob = job;
        while( nextJob )
        {
            pid = fork();
            if( pid == 0 )
            {
                execute_function( job );
                job->done = 1;
                exit(0);
            } else
            {
                nextJob->pid = pid;
                if(nextJob->out_src > 2)
                    close(nextJob->out_src);
                nextJob = nextJob->next;
            }
        }
        nextJob = job;
        while( nextJob )
        {
            if( waitpid(nextJob->pid, 0, 0) == -1)
                error( "ERROR waiting");
            if(nextJob->in_src > 2)
                close(nextJob->in_src);
            nextJob = nextJob->next;
        }
        
        
        if(background) exit(0);
        else freeJob( job );
    }
    
    
}
