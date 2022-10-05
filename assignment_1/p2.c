// TODO : Fix if line is all tabs. 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SZ 256

int main(int argc, char** argv){

    char buf[BUF_SZ];
    char string[BUF_SZ];
    const char* delims = " \t\n";

    fgets(buf, BUF_SZ, stdin);
    strcpy(string, buf);

    char* tokens = strtok(buf,delims);

    while(tokens != NULL){
        printf("%s\n", tokens);
        tokens = strtok(NULL, delims);
    }
    printf("%s", string);
    /*
    int n = 0;

    while(buf[n] != '\n' || buf[n] != '\0'){

        if(buf[n] == '\0' || buf[n] == '\n'){
            if(buf[n - 1] == ' ' || buf[n - 1] == '\t' ||
                buf[n - 1] == '\n' || buf[n - 1] == '\0')
                break;
            putchar('\n');
            break;
        }
            
        else if(buf[n] == ' ' || buf[n] == '\t'){
 
            if(buf[n + 1] == ' ' || buf[n + 1] == '\t' ||
                buf[n + 1] == '\n' || buf[n + 1] == '\0'){
                    n++;
                    continue;
                }
            else{
                putchar('\n');
            }
        }
        else
            putchar(buf[n]);
        n++;
    }
    printf("%s", string);
   */
}