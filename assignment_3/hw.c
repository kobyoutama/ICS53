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
    /* Debug function to check values were properly evicted */
    for (int i=0; i<VM_ADDRS; i++){
        printf("disk_memory[%d] = %d\n", i, disk_memory[i]);
    }
}

void initialize_pages(){
    /* Initalizing pages to zero'ed values */
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
    // prints out memory location and data stored 
    for (int offset=0; offset<PAGE_OFFSET; offset++){
        printf("%d: %d\n", (ppn*PAGE_OFFSET)+offset, main_memory[(ppn*PAGE_OFFSET)+offset].data);
    }
}

void evict(int vm_addr, int mem_addr){
    // evicts main memory pages into disk memory
    for(int i=0; i<PAGE_OFFSET; i++){
        disk_memory[vm_addr+i] = main_memory[mem_addr+i].data;
    }
}


void FIFO(int vm_addr, int data, int rw){
    /* FIFO if page_algo == 0 */
    /* Values in main memory are already checked before coming in */
    // initializes start address for virtual memory
    int start_addr = floor(vm_addr/PAGE_OFFSET)*PAGE_OFFSET;

    // checks if fifo queue is full, if it uses first page to evict 
    if (fifo_queue == MM_ADDRS/PAGE_OFFSET) fifo_queue = 0;

    // checks to see if page is empty
    if (main_memory[(fifo_queue*PAGE_OFFSET)].address != -1) {
        // if not empty, evicts first page and resets bit values
        int p_table_addr = floor(main_memory[(fifo_queue*PAGE_OFFSET)].address/PAGE_OFFSET)*PAGE_OFFSET;
        p_table[p_table_addr].valid_bit = 0;
        p_table[p_table_addr/PAGE_OFFSET].page = p_table_addr/PAGE_OFFSET;

        if (p_table[p_table_addr].dirty_bit){
            evict(main_memory[(fifo_queue*PAGE_OFFSET)].address, fifo_queue*PAGE_OFFSET);
            p_table[p_table_addr].dirty_bit = 0;
        }
    }

    // copy frame from disk to main memory
    for(int i=0; i<PAGE_OFFSET; i++){
        main_memory[(fifo_queue*PAGE_OFFSET)+i].address = start_addr+i;
        
        // checks if it is read or write and will change data/dirty bit if write
        if (!rw && main_memory[(fifo_queue*PAGE_OFFSET)+i].address==vm_addr) {
            main_memory[(fifo_queue*PAGE_OFFSET)+i].data = data;
            p_table[start_addr/PAGE_OFFSET].dirty_bit = 1;
            }
        else
            main_memory[(fifo_queue*PAGE_OFFSET)+i].data = disk_memory[vm_addr+i];
    }
    
    // changes pte values to reflect current status
    p_table[start_addr/PAGE_OFFSET].page = fifo_queue;
    p_table[start_addr/PAGE_OFFSET].valid_bit = 1;

    // increments fifo page
    fifo_queue++;
}

void LRU(int vm_addr, int data, int rw){
    /* LRU if page_algo == 1 */
    /* Values in main memory are already checked before coming in */
    int start_addr = floor(vm_addr/PAGE_OFFSET)*PAGE_OFFSET;
    int page = 0;
    int lru = -999; 
    int lru_pte;

    // finds next empty main memory page or LRU page
    for (int i=0; i<MM_ADDRS/PAGE_OFFSET; i++){
        int vm_addr = floor(main_memory[i*PAGE_OFFSET].address/PAGE_OFFSET)*PAGE_OFFSET;
        if(p_table[vm_addr/PAGE_OFFSET].valid_bit == 0){
            page = i;
            lru = -1;
            break;
        }
        else{
            if (p_table[vm_addr/PAGE_OFFSET].time_stamp > lru){
                lru = p_table[vm_addr/PAGE_OFFSET].time_stamp;
                lru_pte = vm_addr/PAGE_OFFSET;
                page = i;
            }
        }
    }

    // if there is a LRU page, reset bits and evict 
    if(lru != -1){
        p_table[lru_pte].valid_bit = 0;
        p_table[lru_pte].time_stamp = 0;
        p_table[lru_pte].page = lru_pte;
        if(p_table[lru_pte].dirty_bit) {
            evict(main_memory[(page*PAGE_OFFSET)].address, page*PAGE_OFFSET);
            p_table[lru_pte].dirty_bit = 0;
        }
    }

    // copy frame from disk to main memory
    for(int i=0; i<PAGE_OFFSET; i++){
        main_memory[(page*PAGE_OFFSET)+i].address = start_addr+i;

        if (!rw && main_memory[(page*PAGE_OFFSET)+i].address==vm_addr) {
            main_memory[(page*PAGE_OFFSET)+i].data = data;
            p_table[start_addr/PAGE_OFFSET].dirty_bit = 1;
            }
        else

            main_memory[(page*PAGE_OFFSET)+i].data = disk_memory[vm_addr+i];
        
    }

    // changes pte values to reflect current status
    p_table[start_addr/PAGE_OFFSET].page = page;
    p_table[start_addr/PAGE_OFFSET].valid_bit = 1;
    
}
void to_mmem(int va, int num, int rw){
    /* decides what algorithm to use on page faults */
    if(!page_algo){
        FIFO(va, num, rw);
    }
    else{
        LRU(va, num, rw);
    }
}

int check_mmem(int va, int rw){
    /* checks to see if page is in main memory */
    /*      imcrements timestamp for LRU       */
    /*     [read] rw = 1, [write] rw = 0       */
    
    // increment main memory timestamps by 1
    for (int i=0; i<MM_ADDRS/PAGE_OFFSET; i++){
        int vm_addr = floor(main_memory[i*PAGE_OFFSET].address/PAGE_OFFSET)*PAGE_OFFSET;
        if(p_table[vm_addr/PAGE_OFFSET].valid_bit == 1){
            p_table[vm_addr/PAGE_OFFSET].time_stamp += 1; 
        }
    }

    // checks to see if address is in main memory and changes data if it is
    // resets timestamp for LRU if data is changed
    for (int i=0; i<MM_ADDRS; i++){
        if(main_memory[i].address == va){
            int mem_addr = floor(main_memory[i].address/PAGE_OFFSET)*PAGE_OFFSET;
            p_table[mem_addr/PAGE_OFFSET].time_stamp = 0; 

            if (rw) printf("%d\n",main_memory[i].data);
            return 0;
        }
    }

    // prints page fault and moves page into main memory
    printf("A Page Fault Has Occurred\n");
    if (rw){
        to_mmem(va, -1, rw);
        printf("%d\n", disk_memory[va]);
    } 
    return 1;
}

void write_memblock(int va, int num){
    /* writes data into main memory */
    if (check_mmem(va, 0)){
        // Not in memory, calls function to move and write in main memory
        to_mmem(va, num, 0);
    }
    
    else{
        // if in main memory, updates value
        for (int i=0; i<MM_ADDRS; i++){
            if(main_memory[i].address == va){
                main_memory[i].data = num; 
                int start_addr = floor(va/PAGE_OFFSET)*PAGE_OFFSET;
                p_table[start_addr/PAGE_OFFSET].time_stamp = 0;
                p_table[start_addr/PAGE_OFFSET].dirty_bit = 1;
                return;
            }
        }
    }
}



int main(int argc, char **argv){
    initialize_pages();
    if(argc > 1 && strcmp(argv[1], "LRU\n")) page_algo = 1;
    
    while(1){
        char cmd_line[MAX_LINE]={};
        char *args[MAX_TOKENS] = {};
        
        user_input(cmd_line, args);
        
        if (!strcmp(args[0], "quit")) return 0;
        else if (!strcmp(args[0], "read")) check_mmem(atoi(args[1]), 1);
        else if (!strcmp(args[0], "write")) write_memblock(atoi(args[1]), atoi(args[2]));
        else if (!strcmp(args[0], "showmain")) show_memblock(atoi(args[1]));
        else if (!strcmp(args[0], "showptable")) show_pagetable();
        // else if (!strcmp(args[0], "print")) printALL();

    }
    return 0;
}
