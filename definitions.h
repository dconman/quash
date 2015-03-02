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

typedef struct {
char*  function;   // The name/location of the function to be executed
char*  args;       // Any arguments to the function. Can be empty
int    in_src;     // The file descriptor for the input
int    out_src;    // The file descriptor for the output
} command;


void error( const char* err )
{
    dprintf(STDERR_FILENO, "%s", err);
    exit(1);
    
}
