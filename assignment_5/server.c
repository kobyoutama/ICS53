// Koby Outama koutama
/* server.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#define PORT 32222
#define MAX_LINE 256

int server_fd;
struct sockaddr_in address;



void int_handler(int sig){
    shutdown(server_fd, SHUT_RDWR);
    printf("\nServer Closed\n");
    exit(0);
}

void user_input(char *line, char **args){
    /* Parse user inputs */
    printf("> ");                   
    fgets(line, MAX_LINE, stdin); 
    
    char *token = NULL;
    const char* delims = " \t\n";
    
    token = strtok(line, delims);

    while (token != NULL){
        *args = token;
        args++;
        token = strtok(NULL, delims);
    }
}


void start_server(int port){
    /* variables */

    int opt = 1;

    /* create socket */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET,SO_REUSEADDR, &opt,sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    /* call bind */
    if (bind(server_fd, (struct
        sockaddr*)&address,sizeof(address))< 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    /* listen */
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* connection established */
    printf("server started\n");
}

void connection(){
    char buf[256]= { 0 };
    char buffer[256] = { 0 }; 
    int new_socket, valread;
    int addrlen = sizeof(address);
    if ((new_socket= accept(server_fd, (struct
        sockaddr*)&address,(socklen_t*)&addrlen))< 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("New Connection Established\n");
    while(1){
        // change atoi(buffer) to buffer[0] after implementing bytes
        
        if(read(new_socket, buffer, 256)> 0){
        printf("[RECV] sz:%d msg:%s\n", buffer[0], buffer);
        for(int i=0; i<buffer[0]; i++){
            buf[i] = buffer[i+1];
        }
        buf[buf[0]+1] = '\0';
        if(!strcmp(buf, "quit"))
            break;
        }
        send(new_socket, "VALID\0", 6, 0);
        printf("[SENT] VALID\n");
        
   
    }
    close(new_socket);
    printf("Connection Closed\n");
}

int main(int argc, char **argv)
{
    char* pfe_path = argv[1];
    char* mrna_path = argv[2];
    int addrlen;
    int port = PORT; // Fix PORT to atoi(argv[3])
    signal(SIGINT, int_handler);
    start_server(port);
    while(1){
        connection();
    }
    return 0;
}
