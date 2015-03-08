/*****************************************************
 * quash.c
 * 
 * QUite A SHell - by Dawson Conway and Timothy Frese
 * 
 * This program implements a shell which can run
 * executables, as well as four built in functions.
 * It can execute in the forground or background and
 * can redirect I/O between files and programs.
 * 
 * This file contains the
 * parser and main function of quash
 * 
 *****************************************************/

#include "execute.c"

/****************************
 * 
 * Global Variables
 * 
 ****************************/

int std_in;   // always points to stdin, no matter what
int std_out;  // always points to stdout no matter what

/*****************************************************
 * 
 * parse()
 * 
 * parses the input to QUASH, splits it into commands,
 * and passes it to the execute function
 *
 *****************************************************/
void parse()
{
    char* input = malloc(1); // Input read in from the command line
    char* temp;              // Used for string manipulation
    char* in = NULL;         // Pointer to string for input redirect
    char* out = NULL;        // Pointer to string for output redirect
    int append               // Options for opening output file
        = O_CREAT | O_WRONLY;
    int default_permission   // Permissions for creating output file
        = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    char* background;        // Is this command to execute in the background?
    int num_args;            // Holds the number of args for a command
    char** args;             // Holds the args for a command
    int num_pipes;           // Holds the number of pipes in a series of piped commands
    int pipefd[2];           // Used to create pipes
    command* commands;       // The commands to be passed on
    command** tempCommand    // Used to set commands properly
        = &commands;
    size_t size = 1;         // The size of the input
    
    int i,j;                 // For loop variables

    
    if (getline(&input, &size, stdin) == -1) // Read next ine of input, check for errors
        if(errno == 0) exit(0); // This case is the end of file
        else error( "ERROR reading input\n");
        
    if(strcmp( input, "\n") == 0) return; // User entered a blank line
        
    if (size == 0) return; //No input. Shouldn't happen, but need to check
        
    if( background = strchr(input, '&') ) // Is there an '&'? If so bacground execution
    {
        background[0] = 0;                // Remove the &. Assumes it is at end of input
        while((--background)[0] == ' ')   // Remove trailing spaces
            background[0] = 0;
        commands = malloc(sizeof(command));  //This command is a wrapper in case piped commands are executed in the background
        
        size_t length = strcspn( input, "\n");  // get length of input
        commands->function = malloc( length+1 ); // allocate function for wrapper jobs
        memcpy(commands->function, input, length); // The entire command is listed as the function
        commands->function[length] = 0;            // therefore it displays correctly for the jobs command
        
        //null initialize the rest of the command
        commands->args = NULL;
        commands->in_src = -1;
        commands->out_src = -1;

        tempCommand = &(commands->next);// temp command should point to the next command
        
    }
    in = strchr(input, '<'); // Are we redirecting input?
    out = strchr(input, '>'); // Are we redirecting output?
    
    // separate command into components
    if( in ) in[0] = 0;
    if( out ) out[0] = 0;
    
    if( in ) //if we are rdirecting input
    {
        // prevents parser from reading input file as part of command
        in = &in[1]; 
        
        while(in[0] == ' ') // Trim spaces
        {
            in[0]=0;
            in = &in[1];
        }
        
        in[strcspn(in, " \n")] = 0; //end input at space or newline
        
    }
    if( out )
    {
        out = &out[1];
        if(out[0] == '>') // if >> append output instead of overwrite
        {
            append|=O_APPEND;
            out[0]=0;
            out = &out[1];
        }else    // otherwise overwrite
        {
            append|=O_TRUNC;
        }
        
        while(out[0] == ' ') // trim whitespace
        {
            out[0]=0;
            out = &out[1];
        }
        
        out[strcspn(out, " \n")] = 0; //end output at space or newline
    }
    
    // Count number of pipes, and separate string into commands
    temp = input;
    for (num_pipes=0; temp[num_pipes]; temp[num_pipes]=='|' ? temp[num_pipes++]=0 : *temp++);

    // The first input is either stdin or the file name that was passed in
    pipefd[0] = in ? open(in, O_RDONLY) : STDIN_FILENO;
    
    temp = input;
    for(i = 0; i<=num_pipes; i++) //for each command
    {
        (*tempCommand) = malloc(sizeof(command)); 
        
        num_args=1;
        for (j=0; temp[j]; j++) //count the number of spaces
            if (temp[j]==' ')   //this is the upper bound of arguments
                num_args++;
        
        // allocate args, we do this seperately since num_args
        // is an upper bound, we will allocate the exact size
        // for the command
        args = malloc((num_args+1) * sizeof(char*));

        while(temp[0] == ' ') // trim whitespace
        {
            temp[0]=0;
            temp = &temp[1];
        }
        
        size_t length = strcspn( temp, " \n"); //length of first arg
        
        // the first arg is the function we are calling, assign it twice
        (**tempCommand).function = malloc( length+1 );
        memcpy((**tempCommand).function, temp, length);
        (**tempCommand).function[length] = 0;
        
        for( j=0; length > 0 && j < num_args; j++ ) // loop over arguments
        {
            // add a copy of the argument to array
            args[j] = malloc( length+1 );
            memcpy(args[j], temp, length);
            args[j][length] = 0;
            
            // move temp foward to next arg
            temp = &temp[length+1];
            
            while(temp[0] == ' ') //trim whitespace
            {
                temp[0]=0;
                temp = &temp[1];
            }
            length = strcspn( temp, " \n"); //length of next arg 
            
        }
        
        args[j++] = NULL; // execv requires the last argument to be NULL
        
        (**tempCommand).args = malloc(j*sizeof(char*)); // j is the exact number of args
        
        memcpy((**tempCommand).args, args, j*sizeof(char*));// copy the args over
        
        
        // The input was set by the last iteration of the loop
        (**tempCommand).in_src = pipefd[0];
        
        if( i != num_pipes ) // if this is not the last iteration
        {
            if( pipe(pipefd) == -1 ) error( "pipe fail" ); // create a new pipe
            (**tempCommand).out_src = pipefd[1];  // output to the write end of that pipe
                                                  // the next iteration will use the read end
        }
        else // this is the last itteration, use stdout unless we have a redirect
        {
            (**tempCommand).out_src = out ? open( out, append, default_permission) : STDOUT_FILENO;
        }
            
        temp = &(temp[1]); //advance temp 1
        tempCommand = &((**tempCommand).next); //set temp command to the next pointer
        free(args);
    }

    
    (*tempCommand) = NULL; //the last command has a null next pointer
    
    execute( commands, (background? 1 : 0) ); //execute command
    free(input);
}


/**************************************
 * 
 * main(int argc, char** argv)
 * 
 * Initializes the program and contains
 * the infinite loop that parses and
 * check background jobs
 * 
 ***************************************/
int main(int argc, char** argv)
{
    // these ensure we always 
    // have stdin and out
    std_in = dup(STDIN_FILENO); 
    std_out = dup(STDOUT_FILENO);
    
    // This holds bacground jobs, allocate it immediately
    max_jobs = 11;
    background_jobs = malloc(max_jobs*sizeof(command*));

    
    while(1) //Infinite
    {
        // reset STDIN/OUT_FILENO
        dup2(std_in, STDIN_FILENO);
        dup2(std_out, STDOUT_FILENO);
        parse();
        // reset STDIN/OUT_FILENO
        dup2(std_in, STDIN_FILENO);
        dup2(std_out, STDOUT_FILENO);
        checkjobs();
    }
}
