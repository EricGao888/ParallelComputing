//
// Created by Eric Gao on 2020-02-18.
//

//#ifndef HOMEWORK2_QUICKSORT_H
//#define HOMEWORK2_QUICKSORT_H
//
//#endif //HOMEWORK2_QUICKSORT_H

int cmpFunction(const void * a, const void * b);

void check(int *result, int *answer, int len);

void debug(int pos, int rank);

void printArray(int array[], int len);

void MPInitialize(int argc, char *argv[]);

void distribute(int *array, int len);

int getPivot(MPI_Comm comm, int commNum);

int *merge(int *array1, int len1, int *array2, int len2);

void quickSort(int *inputArray);

int binarySearch(int *array, int target, int lh, int rh);

void testBinarySearch();

void gather(int *inputArray);