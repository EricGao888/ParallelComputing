//
// Created by Eric Gao on 2020-02-18.
//

//#ifndef HOMEWORK2_QUICKSORT_H
//#define HOMEWORK2_QUICKSORT_H
//
//#endif //HOMEWORK2_QUICKSORT_H

int cmpFunction(const void * a, const void * b);

void check(int *result, int *answer, int len);

void debug(int pos);

void printArray(int array[], int len);

void MPInitialize(int argc, char *argv[]);

void preprocess(int *array, int len);