#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXTOKENS 64
#define MAXLINE 80
#define BUFFERSIZE 256

void eval(char **args){
    if (!strcmp("pwd", args[0])){
        char dir[BUFFERSIZE];
        printf("%s\n", getcwd(dir, BUFFERSIZE));
    }
    else if (!strcmp("cd", args[0])){
        chdir(args[1]);
    }
}

void get_cmd(char *line, char **args){
    char *token;
    printf("prompt > ");                   
    fgets(line, MAXLINE, stdin); 
    const char* delims = " \t\n";
    
    token = strtok(line, delims);

    while (token != NULL){
        *args = token;
        args++;
        token = strtok(NULL, delims);
    }

}

int main(int argc, char** argv){
    char cmdline[MAXLINE]; 
    char *args[MAXTOKENS]; 
    // char **ptr = args;
    while (1) {
        get_cmd(cmdline, args);
        // printf("%s\n", cmdline);
        // while(*ptr != 0){
        //     printf("%s\n", *ptr);
        //     ptr++;
        // }
        if (feof(stdin))
            exit(0);
        else if (!strcmp("quit", args[0]))
            exit(0);
    
        eval(args);
    } 
}

