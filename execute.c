#include "builtins.c"

void execute_function( command* job )
{
    dup2(job->out_src, STDOUT_FILENO);
    dup2(job->in_src,  STDIN_FILENO);
    
	char* dir = getenv("PWD");
    char* path = getenv( "PATH" );
    int length = strlen(job->function);
    char* function = malloc(length*sizeof(char));
    strcpy(function, job->function);
    char* file = realpath( function, NULL );
    while( file == NULL )
    {
        free(function);
        
        if (path[0]==0)
        {
            printf( "Command not found\n" );
            return;
        }
        
        int pos = strcspn(path, ":");
        file = malloc((length + pos)*sizeof(char));
        memcpy(function, path, pos);
        function[pos]='/';
        strcpy(&(function[pos+1]), job->function);
        path = &(path[pos]);
        if(path[0] == ':') path = &(path[1]);
        
        file = realpath( function, NULL);
    }
    if( execv( file, job->args ) == -1 )
        if( errno != 2 ) error( "Error Executing" );
        
    free(file);

}

void execute( command* job, int background )
{
    int pid = 0;
    command* nextJob;
    
    if( builtin(job) )
    {
        freeJob( job );
        return;
    }
    
    if( background )
    {
        if( (pid = fork()) == -1)
            error( "Error Forking" );
        if(pid != 0)
        {
            job->pid = pid;
            addjob( job );
        }
        job = job->next;
    }
    
    if( pid == 0)
    {
        nextJob = job;
        while( nextJob )
        {
            pid = fork();
            if( pid == 0 )
            {
                execute_function( nextJob );
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
        
        
        if(background)
            exit(0);
        else
            freeJob( job );
    }
    
    
}
