#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

//+
// File:	shell.c
//
// Pupose:	This program implements a simple shell program. It does not start
//		processes at this point in time. However, it will change directory
//		and list the contents of the current directory.
//
//		The commands are:
//		   cd name -> change to directory name, print an error if the directory doesn't exist.
//		              If there is no parameter, then change to the home directory.
//		   ls -> list the entries in the current directory.
//			      If no arguments, then ignores entries starting with .
//			      If -a then all entries
//		   pwd -> print the current directory.
//		   exit -> exit the shell (default exit value 0)
//				any argument must be numeric and is the exit value
//
//		if the command is not recognized an error is printed.
//-

#define CMD_BUFFSIZE 1024
#define MAXARGS 10

int splitCommandLine(char *commandBuffer, char *args[], int maxargs);
int doInternalCommand(char *args[], int nargs);
int doProgram(char *args[], int nargs);

//+
// Function:	main
//
// Purpose:	The main function. Contains the read
//		eval print loop for the shell.
//
// Parameters:	(none)
//
// Returns:	integer (exit status of shell)
//-

int main()
{

    char commandBuffer[CMD_BUFFSIZE];
    // note the plus one, allows for an extra null
    char *args[MAXARGS + 1];

    // print prompt.. fflush is needed because
    // stdout is line buffered, and won't
    // write to terminal until newline
    printf("%%> ");
    fflush(stdout);

    while (fgets(commandBuffer, CMD_BUFFSIZE, stdin) != NULL)
    {
        // printf("%s",commandBuffer);

        // remove newline at end of buffer
        int cmdLen = strlen(commandBuffer);
        if (commandBuffer[cmdLen - 1] == '\n')
        {
            commandBuffer[cmdLen - 1] = '\0';
            cmdLen--;
            // printf("<%s>\n",commandBuffer);
        }

        // split command line into words.(Step 2)

        int nargs = splitCommandLine(commandBuffer, args, MAXARGS);

        // add a null to end of array (Step 2)

        args[nargs] = '\0';

        // debugging
        // printf("%d\n", nargs);
        // int i;
        // for (i = 0; i < nargs; i++){
        //   printf("%d: %s\n",i,args[i]);
        // }
        // element just past nargs
        // printf("%d: %x\n",i, args[i]);

        // TODO: check if 1 or more args (Step 3)
        if(nargs!=0){
        // TODO: if one or more args, call doInternalCommand  (Step 3)
             doInternalCommand(args, nargs);
        // TODO: if doInternalCommand returns 0, call doProgram  (Step 4)
        }
        // TODO: if doProgram returns 0, print error message (Step 3 & 4)
        // that the command was not found.

        // print prompt
        printf("%%> ");
        fflush(stdout);
    }
    return 0;
}

////////////////////////////// String Handling (Step 1) ///////////////////////////////////

//+
// Function:	skipChar
//
// Purpose:	TODO: finish description of function
//
// Parameters:
//    charPtr	Pointer to string
//    skip	character to skip
//
// Returns:	Pointer to first character after skipped chars
//		ID function if the string doesn't start with skip,
//		or skip is the null character
//-

char *skipChar(char *charPtr, char skip)
{
    if (skip == '\0') return charPtr;
    while(*charPtr == skip){
        charPtr++;
    }
    return charPtr;
}



//+
// Funtion:	splitCommandLine
//
// Purpose:	TODO: give descritption of function
//
// Parameters:
//	TODO: parametrs and purpose
//
// Returns:	Number of arguments (< maxargs).
//
//-

int splitCommandLine(char *commandBuffer, char *args[], int maxargs)
{
    int count = 0; 
    for(int i = 0; i < maxargs; i++){ // Ensure that the number of arguments are less than the max number
        commandBuffer = skipChar(commandBuffer, ' '); 
        if(*commandBuffer == '\0') return count;
        args[count] = commandBuffer; // Point args element to the first character in the parameter's word
        count++;
        commandBuffer = strchr(commandBuffer, ' '); // Look for the next space in the buffer
        if(commandBuffer == NULL) return count; // If strchr returned NULL, there is no more spaces
        *commandBuffer = '\0'; // Currently pointing at the space after the word so replace it with a '\0'
        commandBuffer++; // After setting the \0, look at the next character
    }
    fprintf(stderr, "Too many arguments!"); // Number of parameters was larger than allowed
    return 0;

}



////////////////////////////// External Program  (Note this is step 4, complete doeInternalCommand first!!) ///////////////////////////////////

// list of directorys to check for command.
// terminated by null value
char *path[] = {
    ".",
    "/usr/bin",
    NULL};

//+
// Funtion:	doProgram
//
// Purpose:	TODO: add description of funciton
//
// Parameters:
//	TODO: add paramters and description
//
// Returns	int
//		1 = found and executed the file
//		0 = could not find and execute the file
//-

int doProgram(char *args[], int nargs)
{
    // find the executable
    // TODO: add body.
    // Note this is step 4, complete doInternalCommand first!!!

    return 1;
}

////////////////////////////// Internal Command Handling (Step 3) ///////////////////////////////////

// define command handling function pointer type
typedef void (*commandFunc)(char *args[], int nargs);

// associate a command name with a command handling function
struct cmdStruct
{
    char *cmdName;
    commandFunc cmdFunc;
};

// prototypes for command handling functions
void exitFunc(char * args[], int nargs);
void pwdFunc(char * args[], int nargs);


// list commands and functions
// must be terminated by {NULL, NULL}
// in a real shell, this would be a hashtable.
struct cmdStruct commands[] = {
    // TODO: add entry for each command
    {"exit", exitFunc},
    {"pwd", pwdFunc},
    {NULL, NULL} // terminator
};

//+
// Function:	doInternalCommand
//
// Purpose:	TODO: add description
//
// Parameters:
//	TODO: add parameter names and descriptions
//
// Returns	int
//		1 = args[0] is an internal command
//		0 = args[0] is not an internal command
//-

int doInternalCommand(char *args[], int nargs)
{   
    int i = 0;
    while(commands[i].cmdName != NULL){
        if(*commands[i].cmdName == *args[0]){
            commands[i].cmdFunc(args, nargs);
            return 1;
        }
        i++;
    }
    return 0;
}

///////////////////////////////
// comand Handling Functions //
///////////////////////////////

// TODO: a function for each command handling function
// goes here. Also make sure a comment block prefaces
// each of the command handling functions.

void exitFunc(char* args[], int nargs){
    exit(0);
}

void pwdFunc(char* args[], int nargs){
    char *cwd = getcwd(NULL, 0);
    printf("%s\n", cwd);
    free(cwd);
}