#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <ctype.h>


#define historySize 1024
#define variableSize 1024
#define lSize 1024




/*

Name : Liron Cohen
 ID  : 312324247

*/




//FOR SIGNAL
void sig_handler(int sig)
{
  printf("\nYou typed Control-C!\n");
}


//STRUCK 
typedef struct{
    char name[20];
    char value[20];
} var;


//declaration of functuin for read
void readCommand(char** argv, int argc);


// Global variables

char command[lSize];
char commandtmp[lSize];
char commandtmp2[lSize];
char keywords[5];
char thenCommand[lSize];
char elseCommand[lSize];

char error_message[60];

char *saveptr1, *saveptr2;

char ifOutput[lSize];

char path[lSize];
char prompt[lSize] = "hello:";
char history[historySize][lSize];
var variables[variableSize];

char *token, *token2;
int i;
char *infile, *outfile, *errfile;
int isHistory=0;
int fd, amper, redirectIndexes[3], append, retid, status, argc, historyIndex=0, lastStatus;
int fildes[2], fildes2[2];
int originalStdin, originalStdout, originalStderr;
char *argv[30];
int pipeCount;

int* argc_ptr = &argc;

int historyAmount=0;
int isTest=0;

char* cmdptr = command;

char*** pipeArgs;
int* pipeArgsCounts;
void performRedirect();


void readCommand(char** argv, int argc) {
    if (argc != 2) {
        printf("Usage: read <variable_name>\n");
        return;
    }

    char user_value[1024];
    //printf("%s", argv[1]);
     //   printf("Enter value for %s:", argv[1]);
     
    fgets(user_value, sizeof(user_value), stdin);
    user_value[strcspn(user_value, "\n")] = '\0'; // Remove trailing newline if present
	// Store the user input in the specified variable
    for (int i = 0; i < variableSize; i++) {
        if (strcmp(variables[i].name, argv[1]) == 0) {
            strcpy(variables[i].value, user_value);
            return;
        }
    }
    // If variable doesn't exist, create a new one
    for (int i = 0; i < variableSize; i++) {
        if (strcmp(variables[i].name, "") == 0) {
            strcpy(variables[i].name, argv[1]);
            strcpy(variables[i].value, user_value);
            return;
        }
    }
    printf("Error: No space to store variable\n");
}


    // Execute shell commands
    //shellCommand function: This function executes shell commands like cd, prompt, and assignment (=) operations

void shellCommand(char** argv, int argc){
    //is command empty
    if(!argc){
        return;
    }
    
    if(argc == 2 && !strcmp(argv[0], "cd")){
        strcpy(path, "./");
        strcat(path,argv[1]);


        // Attempt to change directory
    if (chdir(path) != 0) {
        // Directory change failed
        perror("cd"); // Print error message
    }
        return;
    }
    

    if(argc == 3 && !strcmp(argv[1], "=")){
        if(!strcmp(argv[0], "prompt")){
            strcpy(prompt, argv[2]);
            return;
        }
        
        if(argv[0][0]=='$'){
            for(int i=0;i<variableSize;i++){
                if(!strcmp(variables[i].name, &argv[0][1]) || !strcmp(variables[i].name, "")){
                    strcpy(variables[i].name, &argv[0][1]);
                    strcpy(variables[i].value, argv[2]);
                    return;
                }
            }
            return;
        }
    }
    
    ///
    
   
   if (argc > 0 && strcmp(argv[0], "read") == 0) {
        readCommand(argv, argc);
        return;
    }
   
   

    
    if(fork() == 0){
        if(!strcmp(argv[0], "echo")){
            if(!strcmp(argv[1], "$?"))
                printf("%d",lastStatus);
            else{
                for(int i=1;i<argc;i++)
                    if(argv[i]!=NULL)
                        printf("%s ",argv[i]);

            }
            printf("\n");
            exit(0);
        }
    
        execvp(argv[0], argv);
        exit(0);
    }
    else{
        while (wait(&status) != -1) {}
    }
    
}


/*
  findRedirectIndexes, getRedirectNames, and performRedirect functions: 
  These functions are responsible for identifying and handling input/output redirection in commands

*/
 
    // Find indexes of redirection operators
    //findAmperIndex function: This function finds the index of the '&' symbol in the command.



void findRedirectIndexes(char** argv, int argc){
    for(int i=0;i<3;i++)
        redirectIndexes[i]=-1;

    for(int i=0;i<argc;i++){
        if(!strcmp(argv[i], "<"))
            redirectIndexes[0]=i;

        if(!strcmp(argv[i], ">") || !strcmp(argv[i],">>"))
            redirectIndexes[1]=i;

        if(!strcmp(argv[i], "2>"))
            redirectIndexes[2]=i;
    }
}

    // Get names for redirection

void getRedirectNames(char** argv, int argc){
    if (redirectIndexes[0]!=-1) {
        infile = argv[redirectIndexes[0]+1];
        argv[redirectIndexes[0]] = NULL;
        argv[redirectIndexes[0]+1] = NULL;

        *argc_ptr = *argc_ptr - 2;
    }

    if (redirectIndexes[1]!=-1) {
        outfile = argv[redirectIndexes[1]+1];

        if(!strcmp(argv[redirectIndexes[1]], ">>"))
            append=1;
        
        argv[redirectIndexes[1]] = NULL;
        argv[redirectIndexes[1]+1] = NULL;

        *argc_ptr = *argc_ptr - 2;
    }

    if (redirectIndexes[2]!=-1) {
        errfile = argv[redirectIndexes[2]+1];
        argv[redirectIndexes[2]] = NULL;
        argv[redirectIndexes[2]+1] = NULL;

        *argc_ptr = *argc_ptr - 2;
    }
}

void performRedirect(){
    /* redirection of IO ? */
    if(redirectIndexes[0]!=-1){
        fd = open(infile,O_RDONLY,0660);
        close(STDIN_FILENO); 
        dup(fd); 
        close(fd); 
    }

    if (redirectIndexes[1]!=-1) {
        if(append)
            fd = open(outfile,O_WRONLY|O_APPEND|O_CREAT,0660); 
        else
            fd = creat(outfile, 0660); 
        
        close (STDOUT_FILENO) ; 
        dup(fd); 
        close(fd);
        
        /* stdout is now redirected */
    } 
    
    if(redirectIndexes[2]!=-1){
        fd = creat(errfile, 0660); 
        close (STDERR_FILENO) ; 
        dup(fd); 
        close(fd); 
    }
}


void redirect(char** argv, int argc){
    findRedirectIndexes(argv, argc);
    getRedirectNames(argv, argc);
    performRedirect();
}



    // Find index of '&' symbol

void findAmperIndex(char** argv, int argc){
    amper=-1;

    for(int i=0;i<argc;i++)
        if(!strcmp(argv[i], "&")){
            amper = i;
            break;
        }
}


/*
parseCommandString, evalCommandString, and trim functions: 
These functions parse, evaluate, and trim the command string, respectively.

*/

    // Parse command string into arguments

void parseCommandString(char* command){
    /* parse command line */
    i = 0;
    isHistory=0;
    token = strtok_r(strcpy(commandtmp, command)," ", &saveptr1);
    while (token != NULL)
    {
        if(!strcmp(token, "!!")){
            isHistory=1;
            int historyIDX = (historyIndex-1)%historySize;
            if(historyIDX<0)
                historyIDX+=historySize;

            token2 = strtok_r(strcpy(commandtmp2, history[historyIDX]), " ", &saveptr2);
            while(token2 != NULL){
                argv[i] = token2;
                token2 = strtok_r(NULL, " ", &saveptr2);
                i++;
                if (token2 && ! strcmp(token2, "|")) {
                    pipeCount++;
                }
            }
        }else{
            argv[i] = token;
            i++;
            if (token && ! strcmp(token, "|")) {
                pipeCount++;
            }
        }
        token = strtok_r(NULL, " ", &saveptr1);
    }
    int equalsIndex = 0;
    for(int j=0;j<i;j++){
        if(!strcmp(argv[j], "=")){
            equalsIndex = j;
            break;
        }
    }

    for(int j=equalsIndex;j<argc;j++){
        if(argv[j]!=NULL && argv[j][0]=='$'){
            for(int k=0;k<variableSize;k++){
                if(!strcmp(&argv[j][1], variables[k].name)){
                    strcpy(argv[j], variables[k].value);
                    break;
                }
            }
        }
            
    }

    argv[i] = NULL;
    argc = i;
}


    // Evaluate command string

void evalCommandString(char* command){
    pipeCount = 0;
    append=0;

    parseCommandString(command);
    if(isHistory){
        for(int i=0;i<argc;i++)
            printf("%s ", argv[i]);
        printf("\n");
    }
    findAmperIndex(argv, argc);

    if(pipeCount){
        if(amper!=-1){
            perror("& not allowed in command with pipes");
            return;
        }
        
        pipeArgs = (char***) calloc(pipeCount+1, sizeof(char**));
        pipeArgsCounts = (int*) malloc((pipeCount+1)*sizeof(int));
        if(!pipeArgs)
            perror("char*** calloc error");

        if(!pipeArgsCounts)
            perror("int* calloc error");
            
        for(int i=0;i<pipeCount+1;i++){
            pipeArgs[i]=(char**)calloc(10, sizeof(char*));
            if(!pipeArgs[i])
                perror("char** calloc error");
        }

        int j=0;
        int k=0;
        char** currentArgs = pipeArgs[j];
        for(int i=0; i<argc; i++){
            if(!strcmp(argv[i], "|")){
                pipeArgsCounts[j]=k;
                currentArgs = pipeArgs[++j];
                k=0;
            }else
                currentArgs[k++]=argv[i];
        }
        pipeArgsCounts[j]=k;
        
        
        int old_stdin = dup(STDIN_FILENO);
        
        for(i=0;i<pipeCount;i++){
            pipe (fildes);
            if (fork() == 0) {
                dup2(fildes[1], 1);
                shellCommand(pipeArgs[i], pipeArgsCounts[i]);
                exit(0);
            }else
                while (wait(&status) != -1) {}

            dup2(fildes[0], 0);
            close(fildes[1]);
        }

        if (fork() == 0) {
            shellCommand(pipeArgs[i], pipeArgsCounts[i]);
            exit(0);
        }else
            while (wait(&status) != -1) {}

        dup2(old_stdin, STDIN_FILENO);
        close(old_stdin);

        for(int i=0;i<pipeCount+1;i++)
            free(pipeArgs[i]);
        

        free(pipeArgs);
        free(pipeArgsCounts);
    }else{
        
        if(amper!=-1){
            if(amper == argc-1){
                argv[argc - 1] = NULL;
                argc--;
            }
            else
                perror("Found & not in final position in command, aborting.");
        }

        originalStdin = dup(STDIN_FILENO);
        originalStdout = dup(STDOUT_FILENO);
        originalStderr = dup(STDERR_FILENO);

        redirect(argv, argc);
        
        shellCommand(argv, argc);

        dup2(originalStdin, STDIN_FILENO);
        dup2(originalStdout, STDOUT_FILENO);
        dup2(originalStderr, STDERR_FILENO);
    }
}

    // Trim leading and trailing non-printable characters

void trim(char* str){
    int l=0;
    int r=strlen(str)-1;
    
    while(l<strlen(str)-1 && !(str[l]>= '!' && str[l] <= '~'))
        l++;
    while(r>=0 && !(str[r]>= '!' && str[r] <= '~'))
        r--;
    
    
    
    // Move characters to the beginning of the string
   // memmove(str, str + l, r - l + 1);
   // str[r - l + 1] = '\0'; // Null-terminate the trimmed string
    
    
    if (l > 0)
        memmove(str, str + l, strlen(str) - l + 1);

    // Null-terminate the trimmed string
    str[r - l + 1] = '\0';
    
   // strcpy(str, &str[l]);
   // str[r-l+1]='\0';
}





//MAIN

int main(int cmdargc, char *cmdargv[]) {


    signal(SIGINT, sig_handler);

    size_t size =lSize;
    
    if(cmdargc>1){
        if(!strcmp(cmdargv[1], "-t"))
            isTest=1;
    }

    int chars=-50;
    
    while (1)
    {   
        chars=-50;
        if(!isTest)
            printf("%s ", prompt);

        historyAmount=0;

        chars = getline(&cmdptr, &size, stdin);
         
        fflush(stdout);
        
       
        
        if(command[0] == '\033'){
            for(int i=0;i<strlen(command)-2;i++){
                if(command[i] == '\033' && command[i+1] == '[')
                    switch(command[i+2]){
                        case 'A':
                            historyAmount++;
                            break;
                        case 'B':
                            if(historyAmount>0)
                                historyAmount--;
                            break;
                    }
                    
            }
            
            if(!historyAmount)
                continue;
            else{
                int hi = (historyIndex - historyAmount)%historySize;
                if(hi<0)
                    hi+=historySize;
                
                strcpy(command, history[hi]);
                
                if(command[0] == '\0'){
                    sprintf(error_message, "No command in history at index %d", hi);
                    perror(error_message);
                    continue;
                }else{
                    printf("\033[1A");
                    printf("\x1b[2K");
                    printf("%s %s\n", prompt , command);
                }
                
            }
        }

        trim(command);

        if(command[0] == 'i' && command[1] == 'f'){
            fgets(keywords,lSize, stdin);

            trim(keywords);
            if(strcmp(keywords, "then")){
                perror("Expected `then` after `if`");
                exit(1);
            }

            fgets(thenCommand,lSize, stdin);

            trim(thenCommand);

            fgets(keywords,lSize, stdin);
            trim(keywords);

            if(strcmp(keywords, "else")){
                perror("Expected `else` after `then`");
                exit(1);
            }

            fgets(elseCommand,lSize, stdin);
            trim(elseCommand);

            fgets(keywords,lSize, stdin);
            trim(keywords);
            if(strcmp(keywords, "fi")){
                printf("keywords: %s\n", keywords);
                perror("Expected `fi` after `else`");
                exit(1);
            }

            strcpy(command, &command[3]);
            pipe(fildes2);
            char tmp[1] = {EOF};
            write(fildes2[1], tmp, 1);
            
            if(fork() == 0){
                dup2(fildes2[1], STDOUT_FILENO);
                evalCommandString(command);
                exit(0);
            }else{
                while (wait(&status) != -1) {}

                int nbytes = read(fildes2[0], ifOutput, sizeof(ifOutput));
                if(nbytes-1)
                    evalCommandString(thenCommand);
                else
                    evalCommandString(elseCommand);
                
            }
        }

        if(!strcmp(command, "quit")){
            
            exit(0);
        }
        evalCommandString(command);

        if(strcmp(command, "!!") && strcmp(history[(historyIndex-1)%historySize], command)){
            strcpy(history[historyIndex], command);
            historyIndex = (historyIndex+1)%historySize;
        }

        lastStatus = status;
    }
}
