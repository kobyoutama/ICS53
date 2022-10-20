#define _POSIX_SOURCE // To get SIG_UNBLOCK defined

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <ctype.h>
#include <signal.h>

#define NOJOB 0
#define BACKGROUND -1
#define FOREGROUND 1
#define STOPPED 2
#define MAXJOBS 5
#define MAXTOKENS 64
#define MAXLINE 80
#define BUFFERSIZE 256

struct jobs{
    char command[MAXLINE];
    pid_t pid;
    int jid;
    int status; // 0 = None, -1 = Background, 1 = Foreground, 2 = Stopped 
};

int fg, currJobs = 0; 
int bg = FOREGROUND;

struct jobs jobList[MAXJOBS]; 
void removeJob(pid_t pid);

void int_handler(int sig){
    int status;
    signal(SIGINT, int_handler);    

    for (int i=0; i<MAXJOBS; i++){
        if(jobList[i].status == FOREGROUND){
            kill(jobList[i].pid, SIGINT);
            waitpid(jobList[i].pid, &status, 0);
            removeJob(jobList[i].pid);
        }
    }
}

void child_handler(int sig){
    pid_t pid;
    int status;
    signal(SIGCHLD, child_handler);  

    // write(STDOUT_FILENO, "Handling Child\n", 16);
    /* Detect any terminated or stopped jobs, but don't wait on others */
    /* Got this section from TA's slide */
    while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0){
        if (WIFSTOPPED(status)){
            printf("status %d terminated with code %d\n", pid, WSTOPSIG(status));
        }
        /* Job terminated (uncaught signal or normally) */
        else if(WIFSIGNALED(status) || WIFEXITED(status)){
            if(WIFEXITED(status))
                printf("status %d terminated with code %d\n", pid, WEXITSTATUS(status));
            else
                printf("status %d signaled with code %d\n", pid, WTERMSIG(status));
            removeJob(pid);
        }
        else
            printf("waitpid error\n");
        fflush(stdout);
    }
}

void stp_handler(int sig){
    // reset stop signal to catch again
    signal(SIGTSTP, stp_handler);

    if(fg > -1){
        for (int i=0; i<MAXJOBS; i++){
            if(jobList[i].status == FOREGROUND){
                kill(jobList[i].pid, SIGTSTP);
                jobList[i].status = STOPPED;
                fg = -1;
            }
        }
    }
}

int addJob(pid_t pid, int status, char* args){
    if (currJobs == MAXJOBS) return -1;
    if (status == FOREGROUND) fg = pid;
    for (int i = 0; i<MAXJOBS; i++){
        if(jobList[i].status == NOJOB){
            // add status
            // printf("[A] JID:%d\t PID:%d\tStat:%d\n" ,i,pid,status);
            strcpy(jobList[i].command, args);
            jobList[i].pid = pid;
            jobList[i].jid = i;
            jobList[i].status = status;
            return 1;
        }
    }
}

void createJobList(){
    for(int i=0; i<MAXJOBS; i++){
        jobList[i].jid = -1;
        jobList[i].pid = 0;
        jobList[i].status = NOJOB;
    }
}

void removeJob(pid_t pid){
    for (int i=0; i<MAXJOBS; i++){
        if (jobList[i].pid == pid)
        {
            jobList[i].jid = -1;
            jobList[i].pid = 0;
            jobList[i].status = NOJOB;  
            currJobs--;
        }
    }
}

void deleteJobs(){
    int status;
    // loops through all jobs and checks status
    for (int i = 0; i < MAXJOBS; i++){
        // if status is not none, kill and reap.
        if(jobList[i].status != NOJOB){
            if(jobList[i].status == STOPPED)
                kill(jobList[i].pid, SIGCONT);

            kill(jobList[i].pid, SIGINT);
            jobList[i].status = NOJOB; 
            pid_t pid = wait(&status);

            if(WIFEXITED(status))
                printf("status %d terminated with code %d\n", pid, WEXITSTATUS(status));
        }
    }
}

void printJobs(){
    // printf("[<job_id>] (<pid>) <status> <command_line>\n");
    for (int i = 0; i < MAXJOBS; i++){
        // if status is not none, kill and reap.
        if(jobList[i].status == BACKGROUND || jobList[i].status == FOREGROUND){
            printf("[%d] (%d) Running %s", jobList[i].jid, jobList[i].pid, jobList[i].command);
        }
        else if(jobList[i].status == STOPPED){
            printf("[%d] (%d) Stopped %s", jobList[i].jid, jobList[i].pid, jobList[i].command);
        }
    }
}

void get_cmd(char *cmdline, char **args){
    /* line: is a string with tabs or spaces to space words
    *  args: pointer to return parsed string
    *  Return:
    *         An array of tokens. 
    */

    char *token = NULL;
    char *line = strcpy(line,cmdline);
    const char* delims = " \t\n";
    
    token = strtok(line, delims);

    while (token != NULL){
        *args = token;
        if(!strcmp(*args, "&")) bg = BACKGROUND;
        args++;
        token = strtok(NULL, delims);
    }
}

void toFG(int id, int idtype){
    // if idtype is pid
    int child_stat;
    if(idtype){
        pid_t pid = id;
         for (int i = 0; i < MAXJOBS; i++){
            if(jobList[i].pid == id){
                jobList[i].status = FOREGROUND;
                fg = jobList[i].pid;
                bg = FOREGROUND;
                
                kill(fg, SIGCONT);
                waitpid(fg,&child_stat,0);
                return;
                
            }
        }
    }
    // if idtype is jid
    else {
        for (int i = 0; i < MAXJOBS; i++){
            if(jobList[i].jid == id){
                jobList[i].status = FOREGROUND;
                fg = jobList[i].pid;
                bg = FOREGROUND;

                kill(fg, SIGCONT);
                waitpid(fg,&child_stat,0);
                return;
                
            }
        }            
    }
}

void terminate(int id, int idtype){
    // if idtype is pid
         int status;

    if(idtype){
        pid_t pid = id;
    // loops through all jobs and checks status
        for (int i = 0; i < MAXJOBS; i++){
        // if status is not none, kill and reap.
            if(jobList[i].pid == id){
                if(jobList[i].status == STOPPED)
                    kill(jobList[i].pid, SIGCONT);

            kill(jobList[i].pid, SIGINT);
            jobList[i].status = NOJOB; 
            pid_t pid = wait(&status);
            removeJob(jobList[i].pid);
            if(WIFEXITED(status))
                printf("status %d terminated with code %d\n", pid, WEXITSTATUS(status));
            }
        }
    }
    // if idtype is jid
    else {
            if(jobList[id].status == STOPPED)
                kill(jobList[id].pid, SIGCONT);

            kill(jobList[id].pid, SIGINT);
            jobList[id].status = NOJOB; 
            pid_t pid = wait(&status);
            removeJob(jobList[id].pid);
            if(WIFEXITED(status))
                printf("status %d terminated with code %d\n", pid, WEXITSTATUS(status));
    }
}

void toBG(int id, int idtype){
    int child_stat;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    if(idtype){
        pid_t pid = id;
         for (int i = 0; i < MAXJOBS; i++){
            if(jobList[i].pid == id){
                jobList[i].status = BACKGROUND;
                bg = FOREGROUND;
                sigprocmask(SIG_BLOCK, &mask, NULL);

                kill(jobList[i].pid, SIGCONT);
                sigprocmask(SIG_UNBLOCK, &mask, NULL);

                return; 
            }
        }
    }
    // if idtype is jid
    else {
        for (int i = 0; i < MAXJOBS; i++){
            if(jobList[i].jid == id){
                jobList[i].status = BACKGROUND;
                bg = FOREGROUND;
                sigprocmask(SIG_BLOCK, &mask, NULL);

                kill(jobList[i].pid, SIGCONT);
                sigprocmask(SIG_UNBLOCK, &mask, NULL);

                return;
            }
        }            
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
    else if (!strcmp("cd", args[0])){
        chdir(args[1]);
        return 1;
    }
    // prints all current jobs (debugging)
    else if (!strcmp("jobs", args[0])){
        printJobs();
        return 1;
    }

    else if (!strcmp("delete", args[0])){
        deleteJobs();
        return 1;
    }
    else if (!strcmp("quit", args[0])){
        deleteJobs();
        exit(0);
    }
    else if (!strcmp("fg", args[0])){
        if (args[1][0] ==  '%'){
            toFG(atoi(strtok(args[1], "%%")), 0);
        }
        else{
            toFG(atoi(args[1]), 1);
        }
        return 1;
    }
     else if (!strcmp("bg", args[0])){
        if (args[1][0] ==  '%'){
            toBG(atoi(strtok(args[1], "%%")), 0);
        }
        else{
            toBG(atoi(args[1]), 1);
        }
        return 1;
    }
    else if (!strcmp("kill", args[0])){
        if (args[1][0] ==  '%'){
            terminate(atoi(strtok(args[1], "%%")), 0);
        }
        else{
            terminate(atoi(args[1]), 1);
        }
        return 1;
    }
    return 0;
}


void eval(){
    int child_stat;
    pid_t pid;
    
    char cmdline[MAXLINE]={};
    char *args[MAXTOKENS] = {};
    
    printf("prompt > ");                   
    fgets(cmdline, MAXLINE, stdin); 
    get_cmd(cmdline, args);

    if (args[0] == NULL) return;

    
    // Figure 8.32 from textbook 
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    // Returns if maxjobs is exceeded i.e. no capacity for jobs
    if (currJobs >= 5){
        printf("ERROR: MAX JOB LIMIT 5\n");
        return;
    }
    // Returns if it is a builtin command i.e. already finished 
    if (builtin_command(args)) return;

    sigprocmask(SIG_BLOCK, &mask, NULL);
    // Creates a fork and continues into child status
    if ((pid = fork()) == 0){
        // child status
            setpgid(0,0); // setting groupid ensures bg status does not terminate from sigint
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
            
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
    // Parent status
    else{
        // Instance where job is foreground
        addJob(pid, bg, cmdline);
        currJobs++;
        if(bg == FOREGROUND){
            waitpid(fg, &child_stat, 0);
            removeJob(fg);
            fg = NOJOB;
        }
        
        // Instance where job is background
        else{
            bg = FOREGROUND;
        }
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }      
}


int main(int argc, char** argv){
    createJobList();
    signal(SIGINT, int_handler);    
    signal(SIGCHLD, child_handler);  
    signal(SIGTSTP, stp_handler);
    while (1) {
        
        if (feof(stdin))
            exit(0);
        
        eval();
    } 
}

