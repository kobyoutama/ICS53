#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>

#define MAXJOBS 5
#define MAXTOKENS 64
#define MAXLINE 80
#define BUFFERSIZE 256

pid_t fg; 
pid_t pid;

void chldint_handler(int sig){
    if(sig == SIGINT)
        exit(0);
    }

void prntint_handler(int sig){
    }

void eval(char **args){
    int child_stat;

    if (!strcmp("pwd", args[0])){
        char dir[BUFFERSIZE];
        printf("%s\n", getcwd(dir, BUFFERSIZE));
    }
    else if (!strcmp("cd", args[0])){
        chdir(args[1]);
    }
    else{
        int err;

        if ((pid = fork()) == 0){

            fg = getpid();
            signal(SIGINT, prntint_handler);

            // child process
            // signal goes here, after fork, before exec
            err = execv(args[0], args);
            err = execvp(args[0], args);
            printf("s");
            exit(err);
        }
        else if(pid == -1){
            printf("ERROR CHILD NOT CREATED\n");
            
        }
        else{
            // parent process
            //kill(pid, SIGINT);
            //wait(NULL);
            waitpid(fg, &child_stat, 0);
            // wait(&child_stat);
            // there is to implement waiting for program
            // if(NULL == argv[i]) waitpid(pid, NULL, 0);
        }      
    }
}

void get_cmd(char *line, char **args){
    /* line: is a string with tabs or spaces to space words
    *  args: pointer to return parsed string
    *  Return:
    *         An array of tokens. 
    */

    char *token;
    printf("prompt >");                   
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
        signal(SIGINT, prntint_handler);

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

