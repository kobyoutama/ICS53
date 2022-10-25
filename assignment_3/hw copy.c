/* <hw.c> */ 

#include <stdio.h>
#include <math.h>
#include <string.h>

#define DEBUG 0
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

int print_mem(int print_type){
    if(print_type == -1){
        for (int i = 0; i<VM_ADDRS; i++){
            printf("VMADDR[%d] : %d\n", i, VM[i]);
        }
    }
    // 0 = print MM/VM
    else if(print_type == 0){
        for (int i = 0; i<VM_ADDRS; i++){
            if(i < MM_ADDRS) printf("MMADDR[%d] : %d ", i, MM[i]);
            printf("VMADDR[%d] : %d\n", i, VM[i]);
        }
    }
    // 1 = print VM
    else if(print_type == 1){
        for (int i = 0; i<VM_ADDRS; i++){
            printf("VMADDR[%d] : %d\n", i, VM[i]);
        }
    }
    // print MM
    else{
        for (int i = 0; i<MM_ADDRS; i++){
            printf("MMADDR[%d] : %d\n", i, MM[i]);
        }
    }

}

int print_page(int print_type){
    if (print_type != -1) printf("[Type] [Page] : [ValidBit] [DirtyBity]\n");
    
    if(print_type == -1){
        for (int i = 0; i<VM_ADDRS/PAGE_SIZE; i++){
            printf("%d:%d:%d:%d\n",
                i, VM_PTE[i].valid_bit, VM_PTE[i].dirty_bit, VM_PTE[i].page);
        }
        return 1;
    }

    /* THESE ARE FOR MY DEBUGGING */
    // 0 = print MM/VM
    else if(print_type == 0){
        for (int i = 0; i<VM_ADDRS/PAGE_SIZE; i++){
            if(i < MM_ADDRS/PAGE_SIZE) printf("MMPAGE[%d] %d : %d %d ",
                i, MM_PTE[i].page, MM_PTE[i].valid_bit, MM_PTE[i].dirty_bit);
            printf("VMPAGE[%d] %d : %d %d\n",
                i, VM_PTE[i].page, VM_PTE[i].valid_bit, VM_PTE[i].dirty_bit);
        }
        return 1;
    }
    // 1 = print VM
    else if(print_type == 1){
        for (int i = 0; i<VM_ADDRS/PAGE_SIZE; i++){
            printf("VMPAGE[%d] %d : %d %d\n",
                i, VM_PTE[i].page, VM_PTE[i].valid_bit, VM_PTE[i].dirty_bit);
        }
        return 1;
    }
    // print MM
    else{
        for (int i = 0; i<MM_ADDRS/PAGE_SIZE; i++){
            printf("MMPAGE[%d] %d : %d %d\n",
                i, MM_PTE[i].page, MM_PTE[i].valid_bit, MM_PTE[i].dirty_bit);        }
        return 1;
    }
    return -1;
}

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

int write_memblock(int va, int num){

}

int print_memblock(int ppn){

}

int main(){
    initialize_pages();
    
    while(1){
        char cmd_line[MAX_LINE]={};
        char *argv[MAX_TOKENS] = {};
        
        user_input(cmd_line, argv);
        
        if (!strcmp(argv[0], "quit")) return 1;
        if (!strcmp(argv[0], "read")) return print_mem(-1);
        if (!strcmp(argv[0], "write")) return write_memblock(argv[1], argv[2]);
        if (!strcmp(argv[0], "showmain")) return showmain(argv[1]);
        if (!strcmp(argv[0], "showptable")) return print_page(-1);


        if (DEBUG) print_memblock(0);
        if (DEBUG) print_page(0);
    }
    return 1;
}
