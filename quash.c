#include "definitions.h"


void parse()
{
    char* input = malloc(1);
    char* temp;
    char* in;
    char* out;
    int append = O_CREAT | O_WRONLY;
    int default_permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    char* background;
    int num_pipes;
    int pipefd[2];
    command* commands;
    size_t size = 1;
    
    int i;
    
    if (getline(&input, &size, stdin) == -1)
    {
        fprintf( stdin, "ERROR reading input\n");
        return;
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
        }
        
        while(out[0] == ' ')
        {
            out[0]=0;
            out = &out[1];
        }
        
        out[strcspn(out, " \n")] = 0;
    }
    
    if( background = strchr(input, '&') )
    {
        background[0] = 0;
    }
    
    temp = input;
    for (num_pipes=0; temp[num_pipes]; temp[num_pipes]=='|' ? temp[num_pipes++]=0 : *temp++);
    
    commands = malloc(sizeof(command) * (num_pipes+1));
    pipefd[0] = in ? open(in, O_RDONLY) : STDIN_FILENO;
    temp = input;
    
    for(i = 0; i<=num_pipes; i++)
    {
        size_t length = strcspn( temp, " \n");
        commands[i].function = malloc( length+1 );
        memcpy(commands[i].function, temp, length);
        commands[i].function[length] = 0;
        
        temp = &temp[length+1];
        length = strcspn( temp, "\n");
        commands[i].args = malloc( length+1 );
        memcpy(commands[i].args, temp, length);
        commands[i].args[length] = 0;
        
        commands[i].in_src = pipefd[0];
        
        if( i != num_pipes )
        {
            if( !pipe(pipefd) ) error( "pipe fail" );
            commands[i].out_src = pipefd[1];
        }
        else
        {
            printf("\n%s\n", out);
            commands[i].out_src = out ? open( out, append, default_permission) : STDOUT_FILENO;
        }
    }
    
    for(i = 0; i<=num_pipes; i++)
        printf( "command: %s\narguments: %s\ninput: %d output: %d\nerrno: %d\n"
                ,commands[i].function, commands[i].args, commands[i].in_src, commands[i].out_src, errno);
    
    free(input);
}



int main(int argc, char** argv)
{
    while(1)
        parse();
}
