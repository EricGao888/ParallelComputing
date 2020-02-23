#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>
#include "QuickSort.h"

int cmpFunction(const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

void check(int *result, int *answer, int len) {
    for (int i = 0; i < len; i++) {
        if (result[i] != answer[i]) {
            printf("Wrong Sorting!\n");
            printf("Result: ");
            printArray(result, len);
            printf("Answer: ");
            printArray(answer, len);
            return;
        }
    }
    printf("Correct!\n");
    fflush(stdout);
}

void debug(int pos, int rank) {
    printf("****************** Debugging log at line %d from rank %d ******************\n", pos, rank);
    fflush(stdout);
}

void printArray(int array[], int len) {
    for (int i = 0; i < len; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
    fflush(stdout);
}

int *merge(int *array1, int len1, int *array2, int len2) {
    int len = len1 + len2;
    int *array = (int *)malloc(len * sizeof(int));
    int p1 = 0, p2 = 0, p3 = 0;
    while (p1 < len1 && p2 < len2) {
        if (array1[p1] < array2[p2]) {
            array[p3++] = array1[p1++];
        }
        else array[p3++] = array2[p2++];
    }
    if (p1 == len1) {
        array1 = array2;
        p1 = p2;
        len1 = len2;
    }
    while (p1 < len1) array[p3++] = array1[p1++];
    return array;
}

int binarySearch(int *array, int lh, int rh, int target) {
    rh += 1;
    while (lh < rh) {
        int mid = lh + ((rh - lh) >> 1);
        if (array[mid] >= target) rh = mid;
        else lh = mid + 1;
    }
    return lh;
}

void testBinarySearch() {
    int array1[] = {1, 2};
    int array2[] = {0, 1, 2};
    printf("%d\n", binarySearch(array1, 0, 1, 0));
    printf("%d\n", binarySearch(array1, 0, 1, 3));
    printf("%d\n", binarySearch(array2, 0, 2, 0));
}