#include "execute.c"


void parse()
{
    char* input = malloc(1);
    char* temp;
    char* in = NULL;
    char* out = NULL;
    int append = O_CREAT | O_WRONLY;
    int default_permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    char* background;
    int num_pipes;
    int pipefd[2];
    int pid = 0;
    command* commands;
    command** tempCommand = &commands;
    size_t size = 1;
    
    int i;
    
    if (getline(&input, &size, stdin) == -1)
        error( "ERROR reading input\n");
        
    if( background = strchr(input, '&') )
    {
        background[0] = 0;
        commands = malloc(sizeof(command));
        
        size_t length = strcspn( input, "\n");
        commands->function = malloc( length+1 );
        memcpy(commands->function, input, length);
        commands->function[length] = 0;
        
        commands->args = NULL;
        
        commands->in_src = -1;
        commands->out_src = -1;
        
        commands->done = 0;
        tempCommand = &(commands->next);
        
    }
    
    if( in = strchr(input, '<') )
    {
        in[0] = 0;
        in = &in[1];
        
        while(in[0] == ' ')
        {
            in[0]=0;
            in = &in[1];
        }
        
        in[strcspn(in, " \n")] = 0;
        
    }
    if( out = strchr(input, '>') )
    {
        out[0] = 0;
        out = &out[1];
        if(out[0] == '>')
        {
            append|=O_APPEND;
            out[0]=0;
            out = &out[1];
        }else
        {
            append|=O_TRUNC;
        }
        
        while(out[0] == ' ')
        {
            out[0]=0;
            out = &out[1];
        }
        
        out[strcspn(out, " \n")] = 0;
    }
    
    temp = input;
    for (num_pipes=0; temp[num_pipes]; temp[num_pipes]=='|' ? temp[num_pipes++]=0 : *temp++);
    
    pipefd[0] = in ? open(in, O_RDONLY) : STDIN_FILENO;
    temp = input;
    
    for(i = 0; i<=num_pipes; i++)
    {
        (*tempCommand) = malloc(sizeof(command));
        
        while(temp[0] == ' ')
        {
            temp[0]=0;
            temp = &temp[1];
        }
        
        size_t length = strcspn( temp, " \n");
        (**tempCommand).function = malloc( length+1 );
        memcpy((**tempCommand).function, temp, length);
        (**tempCommand).function[length] = 0;
        
        temp = &temp[length+1];
        
        while(temp[0] == ' ')
        {
            temp[0]=0;
            temp = &temp[1];
        }
        length = strcspn( temp, "\n");
        (**tempCommand).args = malloc( length+1 );
        memcpy((**tempCommand).args, temp, length);
        (**tempCommand).args[length] = 0;
        
        (**tempCommand).in_src = pipefd[0];
        
        if( i != num_pipes )
        {
            if( pipe(pipefd) == -1 ) error( "pipe fail" );
            (**tempCommand).out_src = pipefd[1];
        }
        else
        {
            (**tempCommand).out_src = out ? open( out, append, default_permission) : STDOUT_FILENO;
        }
        
        (**tempCommand).done = 0;
        tempCommand = &((**tempCommand).next);
    }
    
    (*tempCommand) = NULL;
    
    execute( commands, (background? 1 : 0) );
    
    free(input);
}



int main(int argc, char** argv)
{
    while(1)
    {
        parse();
        checkjobs();
    }
}
