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

        args[nargs] = NULL;

        // //debugging
        // printf("%d\n", nargs);
        // int i;
        // for (i = 0; i < nargs; i++){
        //   printf("%d: %s\n",i,args[i]);
        // }
        // //element just past nargs
        // printf("%d: %x\n",i, args[i]);

        int success;
        // TODO: check if 1 or more args (Step 3)
        if(nargs!=0){ 
            success = doInternalCommand(args, nargs);
        // TODO: if doInternalCommand returns 0, call doProgram  (Step 4)
            if(!success){
                    success = doProgram(args, nargs);
                    // TODO: if doProgram returns 0, print error message (Step 3 & 4)
                    // that the command was not found.
                    if(!success) fprintf(stderr, "Command and/or file not found\n");
            }
        }
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
// Purpose:	TODO: Searches through system to find requested file
//
// Parameters:
//	TODO: The arguments passed in through the command buffer, and the number of arguments that were passed
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

    int i = 0;
    char *cmd_path;
    while(path[i] != NULL){
        cmd_path = (char *) malloc(sizeof(char) * (strlen(path[i]) + strlen(args[0]) + 2));
        sprintf(cmd_path, "%s/%s",  path[i], args[0]);
        struct stat buffer;
        int success = stat(cmd_path, &buffer);
        if(!success){ // Located file
            if(S_ISREG(buffer.st_mode) && (S_IXUSR & buffer.st_mode) != 0){ // Check if file is executable
                break;
            }
        }
        free(cmd_path); // Allocating memory on each iteration of the loop, so make sure to free before doing so
        cmd_path = NULL;
        i++;
    }
    if(cmd_path == 0) return 0; // If cmd_path was freed, then there was no file found
    // if(fork() == 0){ // Only execute this code if it is the child process

    // }

    if(fork() == 0){
        execv(cmd_path, args);
    }
    else{
        wait(NULL);
    }

    free(cmd_path);
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
void cdFunc(char * args[], int nargs);
void lsFunc(char* args[], int nargs);


// list commands and functions
// must be terminated by {NULL, NULL}
// in a real shell, this would be a hashtable.
struct cmdStruct commands[] = {
    // TODO: add entry for each command
    {"exit", exitFunc},
    {"pwd", pwdFunc},
    {"cd", cdFunc},
    {"ls", lsFunc},
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
    while(commands[i].cmdName != NULL){ // Iterate through commands list until we reach NULL
        if(strcmp(commands[i].cmdName, args[0]) == 0){  // Check if args[0] exists in list
            commands[i].cmdFunc(args, nargs); // call command corresponding to args[0] 
            return 1;
        }
        i++; // Look at the next command in the list
    }
    return 0; // args[0] is not an internal command
}

///////////////////////////////
// comand Handling Functions //
///////////////////////////////

/*+
This is the exit function, it simply calls exit(0), exiting the program
-*/
void exitFunc(char* args[], int nargs){
    exit(0); // Exit the program
}

/*+
This is the file select filter, which is used when there is no parameter called with ls
If file name starts with a ., do not include it (return 0) else return 1
-*/
int file_select(const struct dirent *entry)
{
    return entry->d_name[0] != 46; // 46 is ascii for '.'
}

/*+
This is the function for ls
This function first checks if there is an argument passed in, if so it does not use a filter and scans the directory as usual
If there is a parameter, this being the -a parameter, it uses the filter to show the hidden files beginning with a dot
After filtering, it prints the namelist returned by the scandir function
-*/
void lsFunc(char* args[], int nargs){
    struct dirent ** namelist;
    int numEnts;
    if(nargs == 2){ // Check if optional parameter is passed
        if(strcmp(args[1], "-a") == 0) numEnts = scandir(".",&namelist,NULL,NULL); // Do not use filter if -a is passed
        else{
            fprintf(stderr, "Invalid Argument\n");
            return;
        }
    }
    else numEnts = scandir(".",&namelist,file_select,NULL); // If parameter is not passed, use the file_select filter
    int i = 0;
    while(i < numEnts){
            printf("%s  ",namelist[i]->d_name); // Cycle thorugh namelist, and print out file names
            i++;
        }
    printf("\n");
}

/*+
This is the function to print the working directory
It simply uses the get cwd function provided and prints it
-*/
void pwdFunc(char* args[], int nargs){
    char *cwd = getcwd(NULL, 0);
    printf("%s\n", cwd);
    free(cwd);
}

/*+
This is the function for the change directory function
Depending on the parameters, it will change the directory as intended
If there is no parameter passed in, it will change the directory to the home directory
If there is two dots passed in, that takes the directory up one
If there is an actual path passed in, cd will take the user to that entered directory
-*/
void cdFunc(char* args[], int nargs){
    struct passwd *pw = getpwuid(getuid());
    if(pw == NULL){
        fprintf(stderr, "Unable to retrieve pointer to the password entry file\n");
        return;
    }

    char *goTo;
    if(nargs == 1){
        if(pw->pw_dir == NULL) {
            fprintf(stderr, "Unable to find home directory\n");
            return;
        }
        goTo = pw->pw_dir;
    }
    else if(nargs == 2){
        goTo = args[1];
    }
    int success = chdir(goTo); // If more than 2 parameters are passed then the error will be thrown
    if(success != 0){
        fprintf(stderr, "Unable to go to directory\n");
        return;
    } 
}