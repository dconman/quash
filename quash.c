#include "execute.c"

int std_in;
int std_out;

void parse()
{
    char* input = malloc(1);
    char* temp;
    char* in = NULL;
    char* out = NULL;
    int append = O_CREAT | O_WRONLY;
    int default_permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    char* background;
    int num_args;
    char** args;
    int num_pipes;
    int pipefd[2];
    command* commands;
    command** tempCommand = &commands;
    size_t size = 1;
    
    int i,j;

    
    if (getline(&input, &size, stdin) == -1)
        if(errno == 0) exit(0);
        else error( "ERROR reading input\n");
        
    if(strcmp( input, "\n") == 0) return;
        
    if (size == 0) return;
        
    if( background = strchr(input, '&') )
    {
        background[0] = 0;
        while((--background)[0] == ' ')
            background[0] = 0;
        commands = malloc(sizeof(command));
        
        size_t length = strcspn( input, "\n");
        commands->function = malloc( length+1 );
        memcpy(commands->function, input, length);
        commands->function[length] = 0;
        
        commands->args = NULL;
        
        commands->in_src = -1;
        commands->out_src = -1;
        
        tempCommand = &(commands->next);
        
    }
    in = strchr(input, '<');
    out = strchr(input, '>');
    if( in )
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
    if( out )
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
        
        num_args=1;
        for (j=0; temp[j]; j++)
            if (temp[j]==' ')
                num_args++;
        
        args = malloc((num_args+1) * sizeof(char*));

        
        while(temp[0] == ' ')
        {
            temp[0]=0;
            temp = &temp[1];
        }
        
        size_t length = strcspn( temp, " \n");
        (**tempCommand).function = malloc( length+1 );
        memcpy((**tempCommand).function, temp, length);
        (**tempCommand).function[length] = 0;
        
        args[0] = malloc( length+1 );
        memcpy(args[0], temp, length);
        args[0][length] = 0;
        
        temp = &temp[length+1];
        
        while(temp[0] == ' ')
            {
                temp[0]=0;
                temp = &temp[1];
            }
            length = strcspn( temp, " \n");
        
        for( j=1; length > 0 && j < num_args; j++ )
        {
        
            
            args[j] = malloc( length+1 );
            memcpy(args[j], temp, length);
            args[j][length] = 0;
            
            temp = &temp[length+1];
            
            while(temp[0] == ' ')
            {
                temp[0]=0;
                temp = &temp[1];
            }
            length = strcspn( temp, " \n");
            
        }
        
        args[j++] = NULL;
        
        (**tempCommand).args = malloc(j*sizeof(char*));
        
        memcpy((**tempCommand).args, args, j*sizeof(char*));
        
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
            
        temp = &(temp[1]);
        tempCommand = &((**tempCommand).next);
        free(args);
    }

    
    (*tempCommand) = NULL;
    
    execute( commands, (background? 1 : 0) );
    free(input);
}



int main(int argc, char** argv)
{
    std_in = dup(STDIN_FILENO);
    std_out = dup(STDOUT_FILENO);
    
    max_jobs = 11;
    background_jobs = malloc(max_jobs*sizeof(command*));

    
    while(1)
    {
        dup2(std_in, STDIN_FILENO);
        dup2(std_out, STDOUT_FILENO);
        parse();    
        dup2(std_in, STDIN_FILENO);
        dup2(std_out, STDOUT_FILENO);
        checkjobs();
    }
}
