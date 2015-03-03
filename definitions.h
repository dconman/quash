/*********************************************
 * definitions.h
 * 
 * Common definitions for all files in Quash
 * 
 *********************************************/

#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <fcntl.h>
#include "errno.h"

typedef struct command {
char*    function;   // The name/location of the function to be executed
char*    args;       // Any arguments to the function. Can be empty
int      in_src;     // The file descriptor for the input
int      out_src;    // The file descriptor for the output
int      pid;        // The pid if this is a background process
int      done;       // If the job is a background process and has terminated
command* next;       // The next command in a chain of commands
};


void freeJob( command* job )
{
    if( !job->done )
        printf( "ERROR freeing unfinished job\n" );
    
    if( job->next != NULL)
        freeJob(job->next);
    
    if( job->function != NULL)
        free(function);
        
    if( job->args != NULL)
        free(args);
        
    if( job->in_src > -1)
        job->in_src = close(job->in_src) -1;
        
    if( job->out_src > -1)
        job->out_src = close(job->out_src) -1;
        
    free( job );
}


command** background_jobs;
int num_jobs;
int current_jobs;
int max_jobs;

int addjob( command* new_job )
{
    int i;
    for( i=0; i<max_jobs; i++ )
        if(background_jobs[i] = NULL)
        {
            background_jobs[i] = new_job;
            num_jobs++;
            if(num_jobs == max_jobs)
                background_jobs = realloc(background_jobs,
                                        sizeof(command*)*(max_jobs*=2));
                                        
            return i;
        }
    
}

void checkjobs( )
{
    int i;
    for( i=0; i<max_jobs; i++ )
    if( background_jobs[i].done )
    {
        freeJob(background_jobs[i])
        background_jobs[i]=NULL;
        num_jobs--;
    }
}


void error( const char* err )
{
    dprintf(STDERR_FILENO, "%s", err);
    exit(1);
    
}


