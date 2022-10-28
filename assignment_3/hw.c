/* <hw.c> */ 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PAGE_OFFSET 8
#define MAX_TOKENS 8
#define MM_ADDRS 32
#define VM_ADDRS 128
#define MAX_LINE 128


struct Memory{
    int address, data;
};

struct PageBlock
{
    int v_page, page, valid_bit, dirty_bit, time_stamp;
};


int disk_memory[VM_ADDRS] = { [0 ... (VM_ADDRS-1)] = -1 };
struct Memory main_memory[MM_ADDRS];
struct PageBlock p_table[VM_ADDRS/PAGE_OFFSET];

int page_algo, fifo_queue = 0;

void printALL(){
    for (int i=0; i<VM_ADDRS; i++){
        printf("disk_memory[%d] = %d\n", i, disk_memory[i]);
    }
}

void initialize_pages(){
    for (int i = 0; i < VM_ADDRS/PAGE_OFFSET; i++){
        
        p_table[i].page = p_table[i].v_page = i;
        p_table[i].valid_bit = p_table[i].dirty_bit = 0;
        p_table[i].time_stamp = 0;
    }
    for (int i = 0; i < MM_ADDRS; i++){

            main_memory[i].address = -1;
            main_memory[i].data = -1;
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
    /* prints p_table : does not count as use */
    for (int i = 0; i<VM_ADDRS/PAGE_OFFSET; i++){
        printf("%d:%d:%d:%d\n",
            i,
            p_table[i].valid_bit,
            p_table[i].dirty_bit,
            p_table[i].page);
    }
}

void show_memblock(int ppn){
    /* prints VM Block : does not count as use */
    
    // ensures ppn is bounded [0, number of pages]
    if (ppn < 0 || ppn > (MM_ADDRS/PAGE_OFFSET)-1){
        printf("Out Of Bound Main Memory Page : %d\n", ppn);
        return;
    }
    for (int offset=0; offset<PAGE_OFFSET; offset++){
        printf("%d: %d\n", (ppn*PAGE_OFFSET)+offset, main_memory[(ppn*PAGE_OFFSET)+offset].data);
    }
}

void evict(int vm_addr, int mem_addr){
    for(int i=0; i<PAGE_OFFSET; i++){
        disk_memory[vm_addr+i] = main_memory[mem_addr+i].data;
    }
}

// 0
void FIFO(int vm_addr, int data, int rw){
    int start_addr = floor(vm_addr/PAGE_OFFSET)*PAGE_OFFSET;
    if (fifo_queue == MM_ADDRS/PAGE_OFFSET) fifo_queue = 0;
    // TODO : EVICT ONLY WHEN DIRTY BIT == 1, CHANGE VALID BIT TO 0, DO PAGE NUMBER 
    if (main_memory[(fifo_queue*PAGE_OFFSET)].address != -1) evict(main_memory[(fifo_queue*PAGE_OFFSET)].address, fifo_queue*PAGE_OFFSET);
    for(int i=0; i<PAGE_OFFSET; i++){
        main_memory[(fifo_queue*PAGE_OFFSET)+i].address = start_addr+i;
            // printf("PUTTING %d in main_memory[%d]\n", data, fifo_queue+i);

        if (!rw && main_memory[(fifo_queue*PAGE_OFFSET)+i].address==vm_addr) {
            // printf("PUTTING %d in main_memory[%d]\n", data, fifo_queue+i);
            main_memory[(fifo_queue*PAGE_OFFSET)+i].data = data;
            p_table[start_addr/PAGE_OFFSET].dirty_bit = 1;
            }
        else

            main_memory[(fifo_queue*PAGE_OFFSET)+i].data = disk_memory[vm_addr+i];
    }
    
    p_table[start_addr/PAGE_OFFSET].page = (fifo_queue*PAGE_OFFSET);
    p_table[start_addr/PAGE_OFFSET].valid_bit = 1;

    fifo_queue++;
}

// 1
void LRU(int vm_addr){

}
void to_mmem(int va, int num, int rw){
    // FIFO
    if(!page_algo){
        FIFO(va, num, rw);
    }
    else{
        LRU(va);
    }
}

int check_mmem(int va, int rw){
    // [read] rw = 1, [write] rw = 0
    for (int i=0; i<MM_ADDRS; i++){
        if(main_memory[i].address == va){
            if (rw) printf("%d\n",main_memory[i].data);
            return 0;
        }
    }
    printf("A Page Fault Has Occurred\n");
    if (rw){
        to_mmem(va, -1, rw);
        printf("%d\n", disk_memory[va]);
    } 
    return 1;
}

void write_memblock(int va, int num){
    // Not in memory
    if (check_mmem(va, 0)){
        to_mmem(va, num, 0);
    }
    else{
        for (int i=0; i<MM_ADDRS; i++){
            if(main_memory[i].address == va){
                main_memory[i].data = num; 
                int start_addr = floor(va/PAGE_OFFSET)*PAGE_OFFSET;
                p_table[start_addr/PAGE_OFFSET].dirty_bit = 1;
                return;
            }
        }
    }
}



int main(int argc, char **argv){
    initialize_pages();
    if(argc > 1 && strcmp(argv[1], "LRU")) page_algo = 1;
    
    while(1){
        char cmd_line[MAX_LINE]={};
        char *args[MAX_TOKENS] = {};
        
        user_input(cmd_line, args);
        
        if (!strcmp(args[0], "quit")) return 0;
        else if (!strcmp(args[0], "read")) check_mmem(atoi(args[1]), 1);
        else if (!strcmp(args[0], "write")) write_memblock(atoi(args[1]), atoi(args[2]));
        else if (!strcmp(args[0], "showmain")) show_memblock(atoi(args[1]));
        else if (!strcmp(args[0], "showptable")) show_pagetable();
        else if (!strcmp(args[0], "print")) printALL();

    }
    return 0;
}
