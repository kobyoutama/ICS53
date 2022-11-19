// Koby Outama koutama
/* client.c */
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_TOKENS 16
#define BUFFER_SIZE 256
#define PORT 32222

void user_input(char *line, char *buf){
    /* Parse user inputs */
    
    printf("> ");
    fgets(line, BUFFER_SIZE, stdin); 
    line[strcspn(line,"\n")] = '\0';
    
    // limits characters to 255 
    if(strlen(line)>= BUFFER_SIZE-1){
        // flush stdin 
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        printf("Invalid Syntax\n");
        return;
    }

    // concatenates length with characters
    // buf[1] = '\0' ensures string is reset
    buf[0] = strlen(line);
    buf[1] = '\0';
    strcat(buf, line);
}

int main(int argc, char const* argv[])
{
    char buffer[BUFFER_SIZE] = { 0 };
    char cmd_line[BUFFER_SIZE] = { 0 };
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
        
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, "127.0.0.1", 
    &serv_addr.sin_addr)<= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    if ((client_fd= connect(sock, (struct
    sockaddr*)&serv_addr, sizeof(serv_addr)))< 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    user_input(cmd_line, buffer);

    while(1){
        
            send(sock, buffer, BUFFER_SIZE, 0);
            printf("[SENT] message: %s\n", buffer);
            valread = read(sock, buffer, BUFFER_SIZE);
            if(!strcmp(cmd_line,"quit")){break;}
            printf("[RECV] message: %s\n", buffer);
            // printf("%s\n", buffer);
            // printf("ASDASDAD\n");
            user_input(cmd_line, buffer);
    }
    printf("CLOSING CLIENT\n");
    close(sock);
    return 0;
}