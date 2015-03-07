#include "builtins.c"

void execute_function( command* job )
{
    dup2(job->out_src, STDOUT_FILENO);
    dup2(job->in_src,  STDIN_FILENO);
    
	char* dir = getenv("PWD");
    char* path = getenv( "PATH" );
    int length = strlen(job->function);
    char* file = malloc((length)*sizeof(char) );
    strcpy( file, job->function );
    if( execl( file, job->function, job->args, (char *) NULL ) != -1 )
    {
        free(file);
        return;
    }
    if( errno != 2 ) error( "Error Executing" );
    
    
    file = realloc(file, (length+strlen(dir)+1)*sizeof(char));
    
    strcpy( file, dir );
    file[strlen( dir )] = '/';
    strcpy( &(file[strlen(dir)+1]), job->function);
    

    while( execl( file, job->function, job->args, (char *) NULL ) == -1)
    {
        if( errno != 2 || path[0] == 0 ) error( "Error Executing" );
        
        int pos = strcspn(path, ":");

        free(file);
        file = malloc((length + pos)*sizeof(char));
        
        memcpy(file, path, pos);
        file[pos]='/';
        strcpy(&(file[pos+1]), job->function);
        path = &(path[pos]);
        if(path[0] == ':') path = &(path[1]);
          
    }
    
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
                execute_function( job );
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
