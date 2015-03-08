/*********************************************
 * execute.c
 * 
 * Handles execution of the commands
 * 
 *********************************************/
 
 #include "builtins.c"
 
 /*********************************************
 * execute_function( command* job )
 * 
 * Executes an individual function
 * 
 *********************************************/
void execute_function( command* job )
{
    // Set the correct input and output
    dup2(job->out_src, STDOUT_FILENO);
    dup2(job->in_src,  STDIN_FILENO);
    
	char* dir = getenv("PWD"); // Current directory
    char* path = getenv( "PATH" ); // Variables in PATH
    int length = strlen(job->function); // Function length
    char* function = malloc(length*sizeof(char)); // Function to execute
    strcpy(function, job->function);
    char* file                       // Full path to function
        = realpath( function, NULL ); //Check if function is absolute or relative
    while( file == NULL ) // Function was not where we looked for it
    {
        
        // This fails every other time if lenghth of function is 17
        // We have no idea why. Try again
        free(function); 
        
        if (path[0]==0) // We have gotten to the end of path
        {
            printf( "Command not found\n" );
            return;
        }
        
        // set function to the next path / the job function
        int pos = strcspn(path, ":"); // The length of this path variable
        function = malloc((length + pos)*sizeof(char));
        memcpy(function, path, pos);
        function[pos]='/';
        strcpy(&(function[pos+1]), job->function);
        path = &(path[pos]) ;// move the path foward
        if(path[0] == ':') path = &(path[1]); // skip colons
        
        file = realpath( function, NULL); //check if file exists
    }
    if( execv( file, job->args ) == -1 ) //execute the function
        if( errno != 2 ) error( "Error Executing" );
        
    free(file);

}

 /*********************************************
 * execute( command* job )
 * 
 * Set up the command to execute, pass it to
 * the function that will
 * 
 *********************************************/
void execute( command* job, int background )
{
    int pid = 0;  //initialize to 0 so if we don't fork it behaves as child
    command* nextJob;  //used to itterate over jobs
    
    if( builtin(job) ) //This will execute builtins
    {
        freeJob( job );
        return;
    }
    
    if( background ) // Should this run in the background
    {
        if( (pid = fork()) == -1) 
            error( "Error Forking" );
        if(pid != 0) // Parent process
        {
            job->pid = pid;
            addjob( job ); // add job to the background jobs list
        }
        job = job->next; // don't execute wrapper job
    }
    
    if( pid == 0) //child if we forked, otherwise true
    {
        nextJob = job;
        while( nextJob ) // Iterate over jobs
        {
            pid = fork(); // Everything is execute in its own process
            if( pid == 0 )  // child
            {
                execute_function( nextJob );
                exit(0);  // exit after we execute
            } else // parent
            {
                nextJob->pid = pid;
                if(nextJob->out_src > 2)  //if output is a pipe or file
                    close(nextJob->out_src); //close it for this thread
                nextJob = nextJob->next;
            }
        }
        nextJob = job;
        while( nextJob ) // Itterate over jos
        {
            if( waitpid(nextJob->pid, 0, 0) == -1)  //wait for each job
                error( "ERROR waiting");
            if(nextJob->in_src > 2)  //if input is a pipe or file
                close(nextJob->in_src); // close it
            nextJob = nextJob->next;
        }
        
        
        if(background) //is this a background thread?
            exit(0);
        else
            freeJob( job );
    }
    
    
}
