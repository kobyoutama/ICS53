// Koby Outama koutama
/* server.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#define MAX_TOKENS 8
#define MAX_LINE 256
#define NUM_ENTRIES 503

int server_fd;
struct sockaddr_in address;

struct date{
    unsigned int month;
    unsigned int day;
    unsigned int year;
};

struct data{
    struct date date;
    float price; 
};

struct data pfe[NUM_ENTRIES];
struct data mrna[NUM_ENTRIES]; 

void int_handler(int sig){
    shutdown(server_fd, SHUT_RDWR);
    // printf("\nServer Closed\n");
    exit(0);
}

void parse_segment(char *seg, char **args){
    /* char* seg will be a string of 256 characters */
    /* char** will be the return after tokenizing seg */
    int i; 
    char *token = NULL;
    char buf[MAX_LINE] = {0}; 
    const char* delims = " \t\n\0";
    
    strcpy(buf, seg);

    for(i=0; i<seg[0]; i++)
        seg[i] = buf[i+1];

    seg[buf[0]] = '\0';
    printf("%s\n",seg);    
    token = strtok(seg, delims);
    
    while (token != NULL){
        *args = token;
        args++;
        token = strtok(NULL, delims);
    }
}

void store_date(struct date *date, char* str){
    if(sscanf(str, "%d/%d/%d", 
                &date->month,
                &date->day,
                &date->year)==3){
        return;}
    sscanf(str, "%d-%d-%d", 
                &date->year,
                &date->month,
                &date->day);
}

 void read_csv(char* path, struct data list[]){
    /* Adopted from discussion 8 slides */

    FILE* fp = fopen(path, "r");

    char buffer[1024];
    int row = 0;
    int column = 0;

    while(fgets(buffer, 1024, fp)){
        column = 0;
        row++;
        if(row==1)
            continue;

        char* value = strtok(buffer, ",");
        while(value){
            if(column == 0){
                store_date(&list[row-2].date, value);
                // printf("Date :%s", value);
            }
            if(column == 4){
                list[row-2].price = atof(value);
                // printf(" Close:%s", value);   
            }
            value = strtok(NULL, ",");
            column++;
        }
        // printf("\n");
    }  

    fclose(fp);

 }

int comp_date(struct date* date1, struct date* date2){
    /* return 0 if dates match */
    /* returns -1 int if date1 is before date2 */
    /* returns 1 int if date1 is after date2  */
    if(date1->year == date2->year && date2->month == date1->month && date1->day == date2->day){
        return 0;
    }

    if(date1->year <= date2->year){
        if(date1->year == date2->year){
            if(date1->month <= date2->month){
                if(date1->month == date2->month){
                    if(date1->day < date2->day){
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
};

int is_inclusive_date(struct date *date, struct date *start, struct date *end){
    if(comp_date(start, date) == 0 || comp_date(end, date) == 0)
        return 0;
    
    if(comp_date(date, start) == 1 && comp_date(date, end) == -1)
        return 0;

    return -1;
}

void print_date( struct date *date){
    printf("%d/%d/%d",
        date->year,
        date->month,
        date->day);
}

float maxp(char* flag, struct data stock[], struct date *start, struct date *end){
    float answer = 0;
    int buy_day = 0;
    int sell_day = 0;
    int i, j;

    for(i = 0; i<NUM_ENTRIES; i++){
        for(j = 0; j<NUM_ENTRIES; j++){
            if(!is_inclusive_date(&stock[i].date, start, end) && !is_inclusive_date(&stock[j].date, start, end) && comp_date(&stock[i].date, &stock[j].date) == -1){
                    float temp = stock[j].price - stock[i].price;
                    if(!strcmp(flag, "profit")){
                        if(answer < temp){
                            answer = stock[j].price - stock[i].price; 
                        }
                    }
                    else{
                        if(answer > temp){
                            answer = stock[j].price - stock[i].price; 
                        }
                    }
            }
        }
        
    }
    if(!strcmp(flag, "profit"))
        return answer;
    return -answer;
}

float date_stats(struct data stock[], struct date* date){
    int i;
    for(i=0; i<NUM_ENTRIES; i++){
        if(!comp_date(&stock[i].date, date)){
            return stock[i].price;
        }
    }
    return -1;
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

void package_segment(char *line, char* out){
    out[0] = strlen(line);
    int i;
    for (i=0; i<out[0]; i++){
        out[i+1] = line[i];
    }  
    out[out[0]+1] = '\0';

}

void get_data(char **tokens, char *out){
    char buf[MAX_LINE] = { 0 };
    char *command = tokens[0];
    if(!strcmp(command, "PricesOnDate")){
        struct date date; 
        store_date(&date, tokens[1]);
        float pfe_close = date_stats(pfe, &date);
        float mrna_close = date_stats(mrna, &date);
        if(pfe_close != -1 && mrna_close != -1)
            sprintf(buf, "PFE: %.2f | MRNA: %.2f", pfe_close, mrna_close);
        else
            strcat(buf, "Unknown");
    }
    else if(!strcmp(command, "MaxPossible")){
        struct date start;
        struct date end;
        store_date(&start, tokens[3]);
        store_date(&end, tokens[4]);

        float ans;
        char *flag = tokens[1];
        if(!strcmp(tokens[2], "PFE"))
            ans = maxp(flag, pfe, &start, &end);
        else 
            ans = maxp(flag, mrna, &start, &end);
        sprintf(buf, "%.2f", ans);
    }

    package_segment(buf, out);
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
    // printf("New Connection Established\n");
    while(1){

        /* heart of server */

        if(read(new_socket, buffer, 256)> 0){
            char *tokenized[MAX_TOKENS] = { 0 };
            // printf("[RECV] %s\n", buffer);
            parse_segment(buffer, &*tokenized); 

            get_data(tokenized, buffer);

            // printf("[SENT] %s\n", buffer);
            send(new_socket, buffer, MAX_LINE, 0);

        }
        else{
            close(new_socket);
            // printf("Connection Closed\n");
            break;
        }
    }
    
}

int main(int argc, char **argv)
{   
    read_csv(argv[1], pfe);
    read_csv(argv[2], mrna);

    int addrlen;

    int port = atoi(argv[3]); 
    signal(SIGINT, int_handler);
    start_server(port);
    while(1){
        connection();
    }
    return 0;
}
