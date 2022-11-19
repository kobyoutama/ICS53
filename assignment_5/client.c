// Koby Outama koutama
/* client.c */
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <regex.h>

#define MAX_TOKENS 8
#define BUFFER_SIZE 256

int sock = 0;
struct sockaddr_in serv_addr;

int check_date(char *date){
    // pattern for YYYY-MM-DD
    char pat[52] = "^[[:digit:]]\\{4\\}\\(-[[:digit:]]\\{2\\}\\)\\{2\\}$";
    regex_t reegex;
    regcomp(&reegex, pat, 0);

    return regexec(&reegex, date, 0, NULL, 0);
}

int user_input(char *line, char *buf){
    /* Parse user inputs */
    
    printf("> ");
    fgets(line, BUFFER_SIZE, stdin); 
    line[strcspn(line,"\n")] = '\0';
    
    // limits characters to 255 
    if(strlen(line)>= BUFFER_SIZE-1){
        // flush stdin 
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        printf("Invalid syntax\n");
        return -1;
    }

    char *token = NULL;
    char *tokenized[MAX_TOKENS];
    const char* delims = " \t\n";

    int i = 0;
    token = strtok(line, delims);


    while (token != NULL){
        if(i>=5){
            printf("Invalid syntax\n");
            return -1;
        }
        tokenized[i++] = token;
        token = strtok(NULL, delims);
    }
    
    

    if(!strcmp(tokenized[0], "PriceOnDate")){
        if(!check_date(tokenized[1])){
            printf("Correct Format\n");
        }
    }
    else if(!strcmp(tokenized[0], "MaxPossible")){
        if(i == 5 &&
            (!strcmp(tokenized[1], "loss") || !strcmp(tokenized[1], "profit")) &&
            (!strcmp(tokenized[2], "PFE") || !strcmp(tokenized[2], "MRNA")) &&
            !check_date(tokenized[3]) && !check_date(tokenized[4])){
            printf("Correct Format\n");
        }
        else{
            printf("Invalid syntax\n");
            return -1;    
        }
    }
    else if(!strcmp(tokenized[0], "quit")){
        return 0;
    }
    else{
        printf("Invalid syntax\n");
        return -1;
    }
    // concatenates length with characters
    // buf[1] = '\0' ensures string is reset
    buf[0] = strlen(line);
    buf[1] = '\0';
    strcat(buf, line);
    return 0;
}

void connect_to_server(char *src, int port){
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(-1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, src, 
    &serv_addr.sin_addr)<= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(-1);
    }
    
}

void run_client(){
    int valread, client_fd;
    char buffer[BUFFER_SIZE] = { 0 };
    char cmd_line[BUFFER_SIZE] = { 0 };
    
    if ((client_fd= connect(sock, (struct
    sockaddr*)&serv_addr, sizeof(serv_addr)))< 0) {
        printf("\nConnection Failed \n");
        exit(-1);
    }
    printf("Connection Established\n");

    while(1){
        // prompts user input until valid
        while(user_input(cmd_line, buffer)){}
        if(!strcmp(cmd_line,"quit")){break;}
        send(sock, buffer, BUFFER_SIZE, 0);
        printf("[SENT] message: %s\n", buffer);
        valread = read(sock, buffer, BUFFER_SIZE);
        printf("[RECV] message: %s\n", buffer);
    }
    printf("CLOSING CLIENT\n");
    close(sock);
}

int main(int argc, char* argv[])
{
    
    char buffer[BUFFER_SIZE] = { 0 };
    char cmd_line[BUFFER_SIZE] = { 0 };
    while(user_input(cmd_line, buffer)){}


    // establish connection with server
    // char *src = argv[1];
    // int port = atoi(argv[2]);
    char src[64] = "127.0.0.1";
    int port = 32222;

    /* change src and port to arg[2], arg[3]*/
    //connect_to_server(src, port);

    // talk with server
    //run_client();

    return 0;
}