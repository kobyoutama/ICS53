/* <hw.c> */ 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PAGE_SIZE 8
#define MAX_TOKENS 8
#define MM_ADDRS 32
#define VM_ADDRS 128
#define MAX_LINE 128

struct pageBlock
{
    int page;
    char valid_bit, dirty_bit;
};


int MM[MM_ADDRS] = { [0 ... (MM_ADDRS-1)] = -1 };
int VM[VM_ADDRS] = { [0 ... (VM_ADDRS-1)] = -1 };

struct pageBlock VM_PTE[VM_ADDRS/PAGE_SIZE];
struct pageBlock MM_PTE[MM_ADDRS/PAGE_SIZE];

void initialize_pages(){
    for (int i = 0; i < VM_ADDRS/PAGE_SIZE; i++){
        if(i < MM_ADDRS/PAGE_SIZE){
            MM_PTE[i].page = i; 
        }
        VM_PTE[i].page = i; 
    }
}

void user_input(char *line, char **args){
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

void show_pagetable(){
    /* prints VM_PTE : does not count as use */
    for (int i = 0; i<VM_ADDRS/PAGE_SIZE; i++){
        printf("%d:%d:%d:%d\n",
            i,
            VM_PTE[i].valid_bit,
            VM_PTE[i].dirty_bit,
            VM_PTE[i].page);
    }
}

void show_memblock(int ppn){
    /* prints VM Block : does not count as use */
    
    // ensures ppn is bounded [0, number of pages]
    if (ppn < 0 && ppn > VM_ADDRS/PAGE_SIZE) return;

    for (int offset=0; offset<PAGE_SIZE; offset++){
        printf("%d: %d\n", (ppn*PAGE_SIZE)+offset, VM[(ppn*PAGE_SIZE)+offset]);
    }
}

void print_mem(int va){
    
}

void write_memblock(int va, int num){

}

// 0
void FIFO(){

}

// 1
void LRU(){

}

int main(int argc, char **argv){
    initialize_pages();
    int page_algo = 0;
    if(argc > 1 && strcmp(argv[1], "LRU")) page_algo = 1;
    
    while(1){
        char cmd_line[MAX_LINE]={};
        char *args[MAX_TOKENS] = {};
        
        user_input(cmd_line, args);
        
        if (!strcmp(args[0], "quit")) return 0;
        else if (!strcmp(args[0], "read")) print_mem(atoi(args[1]));
        else if (!strcmp(args[0], "write")) write_memblock(atoi(args[1]), atoi(args[2]));
        else if (!strcmp(args[0], "showmain")) show_memblock(atoi(args[1]));
        else if (!strcmp(args[0], "showptable")) show_pagetable();
    }
    return 0;
}
