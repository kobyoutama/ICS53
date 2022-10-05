#include <stdlib.h>
#include <stdio.h>

void madd(int* a, int* b, int m, int n ){
    for (int i = 0; i < m*n; i++) {
        *a += *b;
        *a++;
        *b++;
    }
}

int main(){

    int r = 5, c = 5;
 
    int* arr1 = malloc((r * c) * sizeof(int));
    int* arr2 = malloc((r * c) * sizeof(int));
 
    /* Putting values into in the array */
    for (int i = 0; i < r * c; i++){
        arr1[i] = i + 1;
        arr2[i] = i + 4;
    }

    /* Printing 2D array for visual representation */
    printf("ARRAY 1\n");
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++)
            printf("%d ", arr1[i * c + j]);
        printf("\n");
    }

    printf("ARRAY 2\n");
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++)
            printf("%d ", arr2[i * c + j]);
        printf("\n");
    }
    
    madd(*&arr1, *&arr2, 5, 5);

    printf("ARRAY 1 Modified\n");
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++)
            printf("%d ", arr1[i * c + j]);
        printf("\n");
    }

    free(arr1);
    free(arr2);

    return 0;
}