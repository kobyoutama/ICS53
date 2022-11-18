// Koby Outama koutama
/* client.c */


void user_input(char *line, char **args){
    /* Parse user inputs */
    printf(">");                   
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
