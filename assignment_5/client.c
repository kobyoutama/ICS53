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

int check_valid_dates(char *start, char* end){
    /* return 0 if dates match */
    /* returns -1 int if start is before end */
    /* returns 1 int if start is after end  */
    int sy, sm, sd, ey, em, ed;
    sscanf(start, "%d-%d-%d", &sy, &sm, &sd);
    sscanf(end, "%d-%d-%d", &ey, &em, &ed);

    if(sy == ey && em == sm && sd == ed){
        return 0;
    }

    if(sy <= ey){
        if(sy == ey){
            if(sm <= em){
                if(sm == em){
                    if(sd < ed){
                        return -1;
                    }
                    return 1;
                }
                return -1;
            }
            return 1;
        }
        return -1;
    }
    return 1;

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
    char cpy[BUFFER_SIZE];
    char *tokenized[MAX_TOKENS];
    const char* delims = " \t\n";

    int i = 0;
    strcpy(cpy, line);
    token = strtok(cpy, delims);


    while (token != NULL){
        if(i>=5){
            printf("Invalid syntax\n");
            return -1;
        }
        tokenized[i++] = token;
        token = strtok(NULL, delims);
    }
    
    

    if(!strcmp(tokenized[0], "PricesOnDate")){
        if(!check_date(tokenized[1])){
        }
        else{
                printf("Invalid syntax\n");
                return -1;    
        }
    }   
    else if(!strcmp(tokenized[0], "MaxPossible")){
        if(i == 5 &&
            (!strcmp(tokenized[1], "loss") || !strcmp(tokenized[1], "profit")) &&
            (!strcmp(tokenized[2], "PFE") || !strcmp(tokenized[2], "MRNA")) &&
            !check_date(tokenized[3]) && !check_date(tokenized[4]) &&
            check_valid_dates(tokenized[3], tokenized[4]) == -1){
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
        printf("\nInvalid address/Address not supported \n");
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
    // printf("Connection Established\n");

    while(1){
        // prompts user input until valid
        while(user_input(cmd_line, buffer)){}
        if(!strcmp(cmd_line,"quit")){break;}
        send(sock, buffer, BUFFER_SIZE, 0);
        // printf("[SENT] message: %s\n", buffer);
        valread = read(sock, buffer, BUFFER_SIZE);
        printf("%s\n", buffer);
        // printf("[RECV] message: %s\n", buffer);
    }
    // printf("CLOSING CLIENT\n");
    close(sock);
}

int main(int argc, char* argv[])
{
    // establish connection with server
    char *src = argv[1];
    int port = atoi(argv[2]);

    connect_to_server(src, port);

    // talk with server
    run_client();

    return 0;
}