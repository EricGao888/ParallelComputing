#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "QuickSort.h"

struct node {
    int value;
    struct node *left, *right;
};

struct data {
    int idx, lh, rh, pivot, LH, RH;
    int* inputArray;
    int* helperArray;
    int* fTree;
    int* prefixSums;
    pthread_cond_t* conds;
    pthread_mutex_t* locks;
};

struct quickSortData {
    int lh, rh;
    int *inputArray, *helperArray;
};

int inputArray_len;
int num_threads;
int *sortedArray;
int depth = 0;

int main() {
    srand(time(0));
    struct timeval start, end;

    int *inputArray;
    int *helperArray;
    int *ref_arr;
    num_threads = 1;

    printf("Enter array length: ");
    fflush(stdout);
    scanf("%d", &inputArray_len);
    printf("Enter number of threads: ");
    fflush(stdout);
    scanf("%d", &num_threads);

    inputArray = (int *) malloc(inputArray_len * sizeof(int));
    sortedArray = inputArray;
    helperArray = (int *) malloc(inputArray_len * sizeof(int));
    ref_arr = (int *) malloc(inputArray_len * sizeof(int));

    for (int i = 0; i < inputArray_len; i++) {
        inputArray[i] = rand() % inputArray_len;
        ref_arr[i] = inputArray[i];
    }
    qsort(ref_arr, inputArray_len, sizeof(int), cmpfunc);

    struct quickSortData t;
    t.lh = 0;
    t.rh = inputArray_len - 1;
    t.inputArray = inputArray;
    t.helperArray = helperArray;

    gettimeofday(&start, NULL);
    quickSort(&t);
    gettimeofday(&end, NULL);
    printf("Time use: %ld\n", (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));

    for (int i = 0; i < inputArray_len; i++) {
        if (sortedArray[i] != ref_arr[i]) {
            printf("Wrong Sorting!\n");
            fflush(stdout);
            exit(0);
        }
    }
    printf("Correct!\n");
    fflush(stdout);

//    printf("res: ");
//    printArray(sortedArray, inputArray_len);
//    printf("ref: ");
//    printArray(ref_arr, inputArray_len);
}

void printArray(int array[], int len) {
    for (int i = 0; i < len; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
    fflush(stdout);
}

void initializeArray(int array[], int len) {
    memset(array, -1, len * sizeof(array[0]));
}

int getPrefixSum(int idx, int* fTree, pthread_cond_t* conds, pthread_mutex_t* locks) {
    int res = 0;

    // Compute Exclusive Prefix Sum
    while (idx >= 0) {
        pthread_mutex_lock(&locks[idx]);
        while (fTree[idx] == -1) pthread_cond_wait(&conds[idx], &locks[idx]);
        res += fTree[idx];
        pthread_mutex_unlock(&locks[idx]);
        idx = (idx & (idx + 1)) - 1;
    }

    return res;
}

int getDependencySum(int idx, int* fTree, pthread_cond_t* conds, pthread_mutex_t* locks) {
    // Update fTree (As C array in Fenwick Tree); Signal waiting threads
    int x = idx;
    int y = 1;
    int res = 0;

//    debug(129);
    while ((x & 1) == 1) {
//        debug(123);
        int dependency_idx = idx ^ y;
        pthread_mutex_lock(&locks[dependency_idx]);
        while (fTree[dependency_idx] == -1) pthread_cond_wait(&conds[dependency_idx], &locks[dependency_idx]);
//    debug(163);
        res += fTree[dependency_idx];
        pthread_mutex_unlock(&locks[dependency_idx]);
//        printf("idx ^ y: %d\n", idx ^ y);
        y *= 2;
        x >>= 1;
    }
    return res;
}

// each thread run *prefixSum
void *prefixSumInclusive(void *t) {
//    debug(143);
    struct data *args = t;
    // Compute local sum
    int idx = args->idx;
    int lh = args->lh;
    int rh = args->rh;
    int pivot = args->pivot;
    int *inputArray = args->inputArray;
    int *helperArray = args->helperArray;
    int LH = args->LH;
    int RH = args->RH;
    int* fTree = args->fTree;
    int* prefixSums = args->prefixSums;
    pthread_cond_t* conds = args->conds;
    pthread_mutex_t* locks = args->locks;

    int currentSum = 0;

    for (int i = lh; i <= rh; i++) {
        currentSum += (inputArray[i] > pivot ? 1 : 0);
    }
//    debug(158);
    fTree[idx] = getDependencySum(idx, fTree, conds, locks) + currentSum;
    pthread_cond_broadcast(&conds[idx]);

    prefixSums[idx] = getPrefixSum(idx, fTree, conds, locks);
//    debug(164);
    fflush(stdout);

    int offsetR = idx == 0 ? 0 : prefixSums[idx] - currentSum;
    int offsetL = idx == 0 ? 0 : ((lh - LH) - 1 - (prefixSums[idx] - currentSum)); // definitely correct!
//    printf("lh: %d, rh: %d\n", lh, rh);
    fflush(stdout);
    for (int i = lh; i <= rh; i++) {
        if (inputArray[i] > pivot) {
            helperArray[RH - offsetR] = inputArray[i];
//            printf("inputArray[i]: %d, pos: %d\n", inputArray[i], RH - offsetR);
            fflush(stdout);
            offsetR++;
        }
        else {
            helperArray[LH + offsetL] = inputArray[i];
//            printf("inputArray[i]: %d, pos: %d\n", inputArray[i], LH + offsetL);
            fflush(stdout);
            offsetL++;
        }
    }
}

void *quickSort(void *t) {
    struct quickSortData *args = t;

    int lh = args->lh;
    int rh = args->rh;
    int *inputArray = args->inputArray;
    int *helperArray = args->helperArray;


    // recursion exit
    if (lh == rh) {
        if (inputArray != sortedArray) sortedArray[lh] = inputArray[lh];
        return NULL;
    }
    if (lh > rh) return NULL;

    
    int n = (rh - lh) / num_threads;

    // clear fTree and prefixSums array
    int* fTree = (int*)malloc(num_threads * sizeof(int));
    int* prefixSums = (int*)malloc(num_threads * sizeof(int));
    initializeArray(fTree, num_threads);
    initializeArray(prefixSums, num_threads);

    pthread_cond_t* conds = (pthread_cond_t*)malloc(num_threads * sizeof(pthread_cond_t));
    pthread_mutex_t* locks = (pthread_mutex_t*)malloc(num_threads * sizeof(pthread_mutex_t));

    pthread_t* tids = (pthread_t*)malloc(num_threads * sizeof(pthread_t));

    for (int i = 0; i < num_threads; i++) {
        pthread_mutex_init(&locks[i], NULL);
        pthread_cond_init(&conds[i], NULL);
    }

    struct data* prefixSumArgs = (struct data*)malloc(num_threads * sizeof(struct data));

//    debug(273);
    // create threads
    for (int i = 0; i < num_threads; i++) {

        prefixSumArgs[i].idx = i;
        prefixSumArgs[i].lh = lh + 1 + i * n;
        prefixSumArgs[i].rh = i == num_threads - 1 ? rh : prefixSumArgs[i].lh + n - 1;
        prefixSumArgs[i].pivot = inputArray[lh];
        prefixSumArgs[i].inputArray = inputArray;
        prefixSumArgs[i].helperArray = helperArray;
        prefixSumArgs[i].LH = lh;
        prefixSumArgs[i].RH = rh;
        prefixSumArgs[i].fTree = fTree;
        prefixSumArgs[i].prefixSums = prefixSums;
        prefixSumArgs[i].locks = locks;
        prefixSumArgs[i].conds = conds;

        pthread_create(&tids[i], NULL, prefixSumInclusive, &prefixSumArgs[i]);
//        debug(180);
    }

//    debug(181);
    // join threads
    for (int i = 0; i < num_threads; ++i) {
        int ret = pthread_join(tids[i], NULL);
//        debug(189);
    }

    // rearrange pivot
    int mid = (rh - lh + 1) - 1 - prefixSums[num_threads - 1] + lh;
    helperArray[mid] = inputArray[lh];
    inputArray[mid] = inputArray[lh];

    int *tmp = inputArray;
    inputArray = helperArray;
    helperArray = tmp;

    struct quickSortData lhData, rhData;
    lhData.lh = lh;
    lhData.rh = mid - 1;
    lhData.inputArray = inputArray;
    lhData.helperArray = helperArray;

    rhData.lh = mid + 1;
    rhData.rh = rh;
    rhData.inputArray = inputArray;
    rhData.helperArray = helperArray;

    pthread_t tid;
    pthread_create(&tid, NULL, quickSort, &lhData);
    quickSort(&rhData);

    pthread_join(tid, NULL);

    free(fTree);
    free(prefixSums);
    free(conds);
    free(locks);
    free(tids);
    free(prefixSumArgs);
}

void debug(int pos) {
    printf("****************** Debugging log at line %d ******************\n", pos);
    fflush(stdout);
}

int cmpfunc (const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

// lock obj
// while (condition_variable) wait(condition variable, lock)
// awake, read stuff
// unlock
// broadcast