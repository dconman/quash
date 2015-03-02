/*********************************************
 * definitions.h
 * 
 * Common definitions for all files in Quash
 * 
 *********************************************/


struct command {
char*  function;   // The name/location of the function to be executed
char*  args;       // Any arguments to the function. Can be empty
int    in_src;     // The file descriptor for the input
int    out_src;    // The file descriptor for the output
}
