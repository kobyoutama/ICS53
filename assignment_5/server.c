// Koby Outama koutama
/* server.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE 256

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


int main(int argc, char** argv){
    /* ICS Avaliable ports 30000 - 60000 */
    char* pfe_path = argv[1];
    char* mrna_path = argv[2];
    int port = atoi(argv[3]);

    /* READ FILES AND START SERVER */
    printf("%s %s %d\n", argv[1], argv[2], port);
    printf("server started\n");
    
    return 0;
}
