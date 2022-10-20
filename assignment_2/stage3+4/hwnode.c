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

struct Node{
    pid_t pid;
    struct Node* next;
};

int fg; 
struct Node *head = NULL;
int currjobs = 0;
int bg = 0;

void prntint_handler(int sig){}

void chldint_handler(int sig){

    if(sig == SIGINT && !fg)
        kill(fg, SIGINT);
    }

void child_handler(int sig){
    pid_t pid;
    int status;

    /* Detect any terminated or stopped jobs, but don't wait on others */
    /* Got this section from TA's slide */
    while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0){
        if (WIFSTOPPED(status)){
            printf("Process %d terminated with code %d\n", pid, WSTOPSIG(status));
            fflush(stdout);

        }
        /* Job terminated (uncaught signal or normally) */
        else if(WIFSIGNALED(status) || WIFEXITED(status)){
            if(WIFEXITED(status)){
                printf("Process %d terminated with code %d\n", pid, WEXITSTATUS(status));
                fflush(stdout);
            }
            else{
                printf("Process %d signaled with code %d\n", pid, WTERMSIG(status));
                fflush(stdout);
            }
        }
        else
            printf("waitpid error\n");
            fflush(stdout);
    }
}

void addChild(pid_t pid){
    // Case no nodes
    struct Node* child = malloc(sizeof(struct Node));
    child->pid = pid;

    if (head == NULL){
        head = child;
        child->next = NULL;
    }
    // Case has nodes
    else{
        struct Node *temp = head;
        head = child;
        child->next = temp;
    }

    
}

void deletejobs(){
    struct Node* temp = head;
    while(head != NULL){
        struct Node* temp = NULL;
        temp = head;
        head = head->next;
        kill(temp->pid, SIGINT);
        free(temp);
        currjobs--;
    }
}

void printjobs(){
    struct Node* temp = head;
        printf("%d\n", temp->pid);
    while (temp != NULL){
        printf("%d\n", temp->pid);
        temp = temp->next;
    }
}

int builtin_command(char **args){
    // "pwd" prints present working directory
    if (!strcmp("pwd", args[0])){
        char dir[BUFFERSIZE];
        printf("%s\n", getcwd(dir, BUFFERSIZE));
        return 1;
    }
    // "cd" changes current directory
    // if you change directory then do ls error
    //   "ls: cannot access '': no such file or directory"
    else if (!strcmp("cd", args[0])){
        chdir(args[1]);
        return 1;
    }
    // prints all current jobs (debugging)
    else if (!strcmp("print", args[0])){
        printjobs();
        return 1;
    }

    else if (!strcmp("delete", args[0])){
        deletejobs();
        return 1;
    }
    return 0;
}



void eval(char **args){
    int child_stat;
    pid_t pid;
    // Returns if maxjobs is exceeded i.e. no capacity for jobs
    if (currjobs >= 5){
        printf("ERROR: MAX JOB LIMIT 5\n");
        return;
    }
    // Returns if it is a builtin command i.e. already finished 
    if (builtin_command(args)) return;
    
    // Creates a fork and continues into child process
    if ((pid = fork()) == 0){
        // child process
        currjobs++;
        if(!bg) fg = getpid();
    
        /* IT WORKS UP UNTIL HERE BUDDY */

        else {
            addChild(getpid());
            printf("Child added to background\n");
            printjobs();
        }
            signal(SIGINT, chldint_handler);

            execv(args[0], args);
            execvp(args[0], args);
            printf("ERROR: EXECV[P]() FAILED\n");
            exit(1);
        
    }
    // Returns if creating child was unsuccessful
    else if(pid == -1){
        printf("ERROR: CHILD NOT CREATED\n");
        return;
    }
    // Parent process
    else{
        // Instance where job is foreground
        if(!bg) waitpid(fg, &child_stat, 0);
        
        // Instance where job is background
        else{
            bg = 0;
            printf("HEAD PID: ");
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
    printf("prompt > ");                   
    fgets(line, MAXLINE, stdin); 
    const char* delims = " \t\n";
    
    token = strtok(line, delims);

    while (token != NULL){
        *args = token;
        if(!strcmp(*args, "&")) bg = 1;
        args++;
        token = strtok(NULL, delims);
    }

}

int main(int argc, char** argv){
    char cmdline[MAXLINE]; 

    while (1) {
        char *args[MAXTOKENS] = {};
        signal(SIGINT, prntint_handler);
        get_cmd(cmdline, args);
        
        if (feof(stdin))
            exit(0);
        else if (!strcmp("quit", args[0])){
            deletejobs();
            exit(0);
        }
        
        eval(args);
    } 
}
