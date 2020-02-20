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

struct pair {
    int data;
    int idx;
};

struct quickSortData {
    int lh, rh, num_threads;
    int *inputArray, *helperArray;
};

int inputArrayLen;
int totalThreadsNum;
int *sortedArray;
int depth = 0;

int main() {
    srand(time(0));
    struct timeval start, end;

    int *inputArray;
    int *helperArray;
//    int *ref_arr1;
    int *ref_arr2;

    printf("Enter array length: ");
    fflush(stdout);
    scanf("%d", &inputArrayLen);
    printf("Enter number of threads: ");
    fflush(stdout);
    scanf("%d", &totalThreadsNum);

    printf("Preparing data, please wait...\n");
    fflush(stdout);
    inputArray = (int *) malloc(inputArrayLen * sizeof(int));
    sortedArray = inputArray;
    helperArray = (int *) malloc(inputArrayLen * sizeof(int));
//    ref_arr1 = (int *) malloc(inputArrayLen * sizeof(int));
    ref_arr2 = (int *) malloc(inputArrayLen * sizeof(int));

    for (int i = 0; i < inputArrayLen; i++) {
//        inputArray[i] = rand() % inputArrayLen;
        inputArray[i] = rand();
//        ref_arr1[i] = inputArray[i];
        ref_arr2[i] = inputArray[i];
    }
//    qsort(ref_arr1, inputArrayLen, sizeof(int), cmpfunc);

    struct quickSortData t1, t2;
    t1.lh = 0;
    t1.rh = inputArrayLen - 1;
    t1.inputArray = inputArray;
    t1.helperArray = helperArray;
    t1.num_threads = totalThreadsNum;

    t2.lh = 0;
    t2.rh = inputArrayLen - 1;
    t2.inputArray = ref_arr2;
    t2.helperArray = helperArray;
    t2.num_threads = 1;

    printf("Data preparation done!\n");
    printf("Sorting...\n");
    fflush(stdout);
    gettimeofday(&start, NULL);
    quickSort(&t1);
    gettimeofday(&end, NULL);
    printf("Time use for parallel version: %ld\n", (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    fflush(stdout);

    gettimeofday(&start, NULL);
    quickSort(&t2);
    gettimeofday(&end, NULL);


    for (int i = 0; i < inputArrayLen; i++) {
        if (sortedArray[i] != ref_arr2[i]) {
            printf("Wrong Sorting!\n");
            printf("res: ");
            printArray(sortedArray, inputArrayLen);
            printf("ref: ");
            printArray(ref_arr2, inputArrayLen);
            exit(0);
        }
    }
    printf("Correct!\n");
    printf("Time use for single-thread version: %ld\n", (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    fflush(stdout);

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
//    fflush(stdout);

    int offsetR = idx == 0 ? 0 : prefixSums[idx] - currentSum;
    int offsetL = idx == 0 ? 0 : ((lh - LH) - 1 - (prefixSums[idx] - currentSum)); // definitely correct!
//    printf("lh: %d, rh: %d\n", lh, rh);
    for (int i = lh; i <= rh; i++) {
        if (inputArray[i] > pivot) {
            helperArray[RH - offsetR] = inputArray[i];
//            printf("inputArray[i]: %d, pos: %d\n", inputArray[i], RH - offsetR);
            offsetR++;
        }
        else {
            helperArray[LH + offsetL] = inputArray[i];
//            printf("inputArray[i]: %d, pos: %d\n", inputArray[i], LH + offsetL);
            offsetL++;
        }
    }
}

void *quickSort(void *t) {
    struct quickSortData* args = t;

    int lh = args->lh;
    int rh = args->rh;
    int *inputArray = args->inputArray;
    int *helperArray = args->helperArray;
    int num_threads = args->num_threads;


    // recursion exit
    if (lh == rh) {
        if (inputArray != sortedArray) sortedArray[lh] = inputArray[lh];
        return NULL;
    }

    if (lh > rh) return NULL;

    if ((num_threads <= 1) || (rh - lh < inputArrayLen / totalThreadsNum)) {
//        printf("[lh, rh]: [%d, %d], num of threads: %d\n", lh, rh, num_threads);
//        fflush(stdout);
        qsort(inputArray + lh, rh - lh + 1, sizeof(int), cmpfunc);
        for (int i = lh; i <= rh; i++) sortedArray[i] = inputArray[i];
        return NULL;
    }

    struct pair pairs[2000];
    for (int i = 0; i < 2000; i++) {
        pairs[i].idx = (rand() % (rh - lh + 1)) + lh;
        pairs[i].data = inputArray[pairs[i].idx];
    }

    qsort(pairs, 2000, sizeof(struct pair), cmpPair);

    int x = inputArray[lh];
    inputArray[lh] = pairs[1000].data;
    inputArray[pairs[1000].idx] = x;

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
    lhData.num_threads = (int)((1.0 * num_threads * (mid - lh) / (rh - lh + 1)) + 0.5);

    rhData.lh = mid + 1;
    rhData.rh = rh;
    rhData.inputArray = inputArray;
    rhData.helperArray = helperArray;
    rhData.num_threads = num_threads - lhData.num_threads;

//    printf("left: [%d, %d], right: [%d, %d]\n", lhData.lh, lhData.rh, rhData.lh, rhData.rh);
//    printf("[lh, rh]: [%d, %d], num of threads: %d\n", lh, rh, num_threads);
//    fflush(stdout);

    pthread_t tid;
    pthread_create(&tid, NULL, quickSort, &lhData);
    quickSort(&rhData);

    pthread_join(tid, NULL);
//
//
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

int cmpfunc(const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

int cmpPair(const void * a, const void * b) {
    return (((struct pair*)a)->data - ((struct pair*)b)->data);
}

// lock obj
// while (condition_variable) wait(condition variable, lock)
// awake, read stuff
// unlock
// broadcast


#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 4

int main(int argc, char *argv[])
{
    int rank, size;     // for storing this process' rank, and the number of processes
    int *sendcounts;    // array describing how many elements to send to each process
    int *displs;        // array describing the displacements where each segment begins

    int rem = (SIZE*SIZE)%size; // elements remaining after division among processes
    int sum = 0;                // Sum of counts. Used to calculate displacements
    char rec_buf[100];          // buffer where the received data should be stored

    // the data to be distributed
    char data[SIZE][SIZE] = {
            {'a', 'b', 'c', 'd'},
            {'e', 'f', 'g', 'h'},
            {'i', 'j', 'k', 'l'},
            {'m', 'n', 'o', 'p'}
    };

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *sendcounts = malloc(sizeof(int)*size);
    int *displs = malloc(sizeof(int)*size);

    // calculate send counts and displacements
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (SIZE*SIZE)/size;
        if (rem > 0) {
            sendcounts[i]++;
            rem--;
        }

        displs[i] = sum;
        sum += sendcounts[i];
    }

    // print calculated send counts and displacements for each process
    if (0 == rank) {
        for (int i = 0; i < size; i++) {
            printf("sendcounts[%d] = %d\tdispls[%d] = %d\n", i, sendcounts[i], i, displs[i]);
        }
    }

    // divide the data among processes as described by sendcounts and displs
    MPI_Scatterv(&data, sendcounts, displs, MPI_CHAR, &rec_buf, 100, MPI_CHAR, 0, MPI_COMM_WORLD);

    // print what each process received
    printf("%d: ", rank);
    for (int i = 0; i < sendcounts[rank]; i++) {
        printf("%c\t", rec_buf[i]);
    }
    printf("\n");

    MPI_Finalize();

    free(sendcounts);
    free(displs);

    return 0;
}