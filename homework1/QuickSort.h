//#ifndef "QuickSort.h"
//#define "QuickSort.h"

void printArray(int array[], int len);

void initializeArray(int array[], int len);

int getDependencySum(int idx, int* fTree, pthread_cond_t* conds, pthread_mutex_t* locks);

int getPrefixSum(int idx, int* fTree, pthread_cond_t* conds, pthread_mutex_t* locks);

int cmpfunc(const void * a, const void * b);

void *prefixSumInclusive(void *t);

void *quickSort(void *t);

void debug(int pos);

int cmpfunc (const void * a, const void * b);

//#endif