/* <hw4.c> */


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MIN_SPLIT 3
#define MAX_TOKENS 8
#define HEAP_SIZE 127
#define MAX_LINE 128


int fit_algo = 0;
int block_ct = 1;
char heap[HEAP_SIZE] = { [0 ... (HEAP_SIZE-1)] = 0 };

struct BlockList{
    int size;
    int start;
    struct BlockList *next; 
};

int msb(int i){
    /* function returns value of the 7 most significant bits in decimal */
    return (i>>1) & 0x7F;
}

void to_bits(int n){
    /* adopted from "https://www.geeksforgeeks.org/binary-representation-of-a-given-number/" */
    /*     for debugging to ensure correct bit manipulation */
    unsigned i;
    for (i = 1 << 7; i > 0; i = i / 2)
        (n & i) ? printf("1") : printf("0");
    printf("\n");
}

void print_heap(){
    /* DEBUG FUNCTION : prints heap memory*/
    for (int i=0; i<HEAP_SIZE; i++){
        printf("%d : %d\n", i, heap[i]);
    }
}

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

void add_block(int p, int len){
    /* Function adds a block of length "len" given a pointer "p" */
    int newsize = ((len+2)<<1);       // calculate bits of new size
    int oldsize = heap[p]&-2;         // mask out low bit
    heap[p] = newsize|1;              // set new length
    
    if(msb(oldsize)-msb(newsize) >= MIN_SPLIT){
        // set length in remaining part of block (footer)
        heap[p+msb(newsize)-1] = newsize|1; 

        // splits block into two
        int newfree = ((msb(oldsize)-msb(newsize))<< 1) &-2;
        heap[p+msb(newsize)] = newfree;
        heap[p+msb(oldsize)-1] = newfree;
        block_ct += 1;
    }
    else{
        // set length in remaining part of block (footer)
        heap[p+msb(oldsize)-1] = newsize|1; 
    }
}

void deallocate(int p){
    /* p is the start of a payload (p-1) == header */

    // size of block (not including header/footer)
    int sz = msb(heap[p-1])-2;

    // first set all values to 0
    for (int i=0; i<sz; i++)
        heap[p+i] = 0;
    
    // sets header/footer as free block
    heap[p+sz] = heap[p-1] = ((sz+2) << 1) & -2 ;

    // forward coalesce if next block is free
    int next_head = p+sz+1;
    
    // checks not out of bound of heap
    if (next_head < HEAP_SIZE-1){
        // checks if next block is free
        if(!(heap[next_head] & 1)){
            // if free, coalesce blocks
            block_ct -= 1; 
            int next_sz = msb(heap[next_head])-2;
            heap[next_head] = heap[next_head-1] = 0;

            int new_sz = ((next_sz+sz+4) << 1) & -2;
            heap[p-1] = new_sz;
            heap[next_head+next_sz+1] = new_sz;
            
            // updates current block's size 
            sz = msb(new_sz) -2;
        }
    }

    // backward coalesce if previous block is free
    int prev_footer = p-2; 

    // checks not out of bound of heap
    if (prev_footer > 0){
        // checks if previous block is free
        if(!(heap[prev_footer] & 1)){
            // if free, coalesce blocks
            block_ct -= 1; 
            int prev_sz = msb(heap[prev_footer])-2;
            heap[prev_footer] = heap[prev_footer+1] = 0;

            int new_sz = ((prev_sz+sz+4) << 1) & -2;
            heap[prev_footer-prev_sz-1] = new_sz;
            heap[p+sz] = new_sz;
        }
    }
}


void allocate(int size){
    /* Function allocates a block of size "size" into heap */

    int p = 0; // starts ptr at index 0
    /* Allocate using First Fit */
    if (!fit_algo){
        // printf("First Fit\n");
        while ((p < HEAP_SIZE) && ((heap[p] & 1) || msb(heap[p]) < size))   
            // sets that something is found and iterate to next block
            p = p + (msb(heap[p] & -2));
        
        // only add block if there is an open block
        if(p < HEAP_SIZE-1 && size <= HEAP_SIZE-2){
            add_block(p, size);  
            printf("%d\n",p+1);
            return;
        }
    }
    /* Allocate using Best Fit */
    else{
        //printf("Best Fit\n");
        int smallest_size = HEAP_SIZE+1;
        int smallest_p = -1;

        // cycle through entire heap
        while(p<HEAP_SIZE){
            // check if it is unallocated and the size < unallocated block size
            if((!(heap[p]&1)) && size < msb(heap[p])){
                // if the current unallocated block is smaller than the saved block
                if(smallest_size > msb(heap[p])){
                    // remember pointer location and size
                    smallest_size = msb(heap[p]);
                    smallest_p = p;
                }
            }
            // iterate to next block
            p = p + (msb(heap[p] & -2));
        }

        // if pointer is -1, could not find a block, so skip adding and print error 
        if(smallest_p != -1 && size <= HEAP_SIZE-2){
            add_block(smallest_p, size);  
            printf("%d\n",smallest_p+1);
            return;
        }
    }  
    printf("Heap Full : Unsuccessful Malloc of Size :%d\n", size);
}

void write_mem(int p, char* str){
    /* function writes ascii value of characters into heap */
    /*     *** Does not do explicit bounds-checking ***    */
    for (int i=0; i<strlen(str); i++)
        heap[p+i] = (int) str[i]; 
}

void print_mem(int p, int len){
    /*     function prints decimal value of heap memory    */
    /*     *** Does not do explicit bounds-checking ***    */
    for(int i=0; i<len; i++)
        (i == 0) ? printf("%d",heap[p]) : printf("-%d", heap[p+i]);
    printf("\n");
}

void blocklist(){
    /* Function prints list of blocks ordered by payload size */

    int p = 0;
    struct BlockList *head = NULL;

    // Loop through heap
    while(p<HEAP_SIZE){

        // Create linked list of blocks
        struct BlockList* new_block = malloc(sizeof(struct BlockList));
        new_block->size = heap[p];
        new_block->start = p;
        new_block->next = NULL;

        // Creates the link list if first block
        if(head == NULL){
            head = new_block;
        }
        // Adds block as the new head if the new block size is < head size
        else if(msb(new_block->size) > msb(head->size)){
            new_block->next = head;
            head = new_block;
        }
        // For cases after first block is added into linked list
        else {
            struct BlockList* temp = head;
            while(temp != NULL){
                // Adds block to end of the linked list
                if(msb(new_block->size) <= msb(temp->size) && temp->next == NULL){
                    temp->next = new_block;
                    break;
                }
                // Adds new block inbetween blocks i.e. temp -> new block -> (temp->next)
                else if(msb(new_block->size) <= msb(temp->size) &&
                  msb(new_block->size) > msb(temp->next->size)){
                    new_block->next = temp->next;
                    temp->next = new_block;
                    break;
                }
                temp = temp->next;
            }
        }
        // go to new block
        p = p + (msb(heap[p] & -2));
    }

    // print out order by decending order on payload size
    while(head!=NULL){
        if (!(head->size&1))
            printf("%d-%d-free\n",msb(head->size)-2,head->start+1);
        else
            printf("%d-%d-allocated\n",msb(head->size)-2,head->start+1);

        head = head->next;
    }

    // free all blocks in linked list
    while(head != NULL){
        struct BlockList* temp = NULL;
        temp = head;
        head = head->next;
        free(temp);
    }
}

void initialize_heap(){
    // set header/footer size of heap and LSB to 0 
    int size = ((HEAP_SIZE) << 1) & -2; 
    heap[0] = size;
    heap[HEAP_SIZE-1] = size; 
}


int main(int argc, char **argv){
    initialize_heap();

    if(argc > 1 && strcmp(argv[1], "BestFit\n")) fit_algo = 1;
    
    while(1){
        char cmd_line[MAX_LINE]={};
        char *args[MAX_TOKENS] = {};
        
        user_input(cmd_line, args);
        
        if (!strcmp(args[0], "quit")) return 0;
        else if (!strcmp(args[0], "malloc")) allocate(atoi(args[1]));
        else if (!strcmp(args[0], "free")) deallocate(atoi(args[1]));
        else if (!strcmp(args[0], "writemem")) write_mem(atoi(args[1]),args[2]);
        else if (!strcmp(args[0], "printmem")) print_mem(atoi(args[1]),atoi(args[2]));
        else if (!strcmp(args[0], "blocklist")) blocklist();
        
        // Functions below are meant for debugging purposes
        else if (!strcmp(args[0], "print")) print_heap();
        else if (!strcmp(args[0], "printb")) to_bits(heap[atoi(args[1])]);

    }
    return 0;
}
