#include "definitions.h"

void execute_function( command* job )
{
    dup2(job->out_src, STDOUT_FILENO);
    dup2(job->in_src,  STDIN_FILENO);
    if( !builtin( job ) )
        execl( job->function, job->function, job->args, (char *) NULL );
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
            waitpid(nextJob->pid, 0, 0);
            if(nextJob->in_src > 2)
                close(nextJob->in_src);
            nextJob = nextJob->next;
        }
        
        
        if(background) exit(0);
    }
    
    
}
