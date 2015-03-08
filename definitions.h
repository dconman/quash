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
#include "signal.h"

 /*********************************************
 * error( const char* err )
 * 
 * Prints out error and exits
 * 
 *********************************************/
void error( const char* err )
{
    dprintf(STDERR_FILENO, "%s %d\n", err, errno);
    exit(1);
    
}


 /*********************************************
 * 
 * The struct representing a job
 * 
 *********************************************/
typedef struct command command;
struct command {
char*    function;   // The name/location of the function to be executed
char**   args;       // Any arguments to the function. Can be empty
int      in_src;     // The file descriptor for the input
int      out_src;    // The file descriptor for the output
int      pid;        // The pid if this is a background process
command* next;       // The next command in a chain of commands
};

 /*********************************************
 * freeJob( command* job )
 * 
 * recursively free all elements of a job
 * 
 *********************************************/
void freeJob( command* job )
{
    int i;
    
    if( job->next != NULL)
        freeJob(job->next);
    
    if( job->function != NULL)
        free( job->function);
        
    if( job->args != NULL)
    {
        for(i=0; job->args[i]; i++)
            free( job->args[i] );
        free(job->args);
    }
        
    if( job->in_src > -1)
        job->in_src = close(job->in_src) -1;
        
    if( job->out_src > -1)
        job->out_src = close(job->out_src) -1;
        
    free( job );
}

 /*********************************************
 * 
 * background jobs variables
 * 
 *********************************************/
command** background_jobs;
int num_jobs;
int max_jobs;

 /*********************************************
 * addjob( command* new_job )
 * 
 * adds a job to the background list
 * 
 *********************************************/
int addjob( command* new_job )
{
    int i;
    for( i=0; i<max_jobs; i++ ) //search for an empty slot in the array
        if(background_jobs[i] == NULL) //found an empty slot
        {
            background_jobs[i] = new_job; // assign it the value
            num_jobs++;
            if(num_jobs == max_jobs) //if the array is full double the size
                background_jobs = realloc(background_jobs,
                                        sizeof(command*)*(max_jobs*=2));
            
            dprintf( 0, "[%d] %d running in background\n", i, new_job->pid );
            return i;
        }
    
}

 /*********************************************
 * checkjobs( )
 * 
 * Checks for background jobs that have
 * terminated and clean them up
 * 
 *********************************************/
void checkjobs( )
{
    int i;
    for( i=0; i<max_jobs; i++ ) // Loop through job arrays
        if( background_jobs[i] != NULL ) 
        {
            int status;
            waitpid( background_jobs[i]->pid, &status, WNOHANG); //finish zombie children
            
            // Test if job is dead
            if ( (kill(background_jobs[i]->pid, 0) == 0)) continue;
            if( errno != ESRCH ) continue; //Other errors
            
            printf("[%d] %d finished %s\n",i, background_jobs[i]->pid, background_jobs[i]->function );
            freeJob(background_jobs[i]);
            background_jobs[i]=NULL;
            num_jobs--;
        }
}
