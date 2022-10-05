#include <stdio.h>
#include <stdlib.h>

#define ROLLS 100

int main() {
    int random_num;
    int num_rolls[11] = {0}; 
    
    for(int roll=0; roll<100; roll++){
        random_num = rand()/(RAND_MAX/6) + 1;
        random_num += rand()/(RAND_MAX/6) + 1;
        num_rolls[random_num-2]++; 
    } 
    
    for(int i=0; i<11; i++){
        printf("%d: %d\t", i+2, num_rolls[i]);
        for(int j=0; j<num_rolls[i]; j++){
            printf("*");
        }
        printf("\n");
    }
}
