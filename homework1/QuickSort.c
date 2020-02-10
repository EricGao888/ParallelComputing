#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
// lock obj
// while (condition_variable) wait(condition variable, lock)
// awake, read stuff
// unlock
// broadcast

struct node {
    int value;
    struct node *left, *right;
};

struct data {
    int idx, lh, rh, pivot;
};

int *nums;
int *nums0;
int nums_len;
int num_threads;


int prefixSums[128];
int fTree[128];
struct data args_arr[128];

pthread_cond_t conds[128];
pthread_mutex_t locks[128];
pthread_t tids[128];


//void *sayHi(void *args);

//void testMultithread();

void singleQuickSort(int lh, int rh, int nums[]);

void testSingleQuickSort();

void printArray(int array[], int len);

void initializeArray(int array[], int len);

int getDependencySum(int idx);

int getPrefixSum(int idx);

void *prefixSumInclusive(void *t);

void quickSort(int lh, int rh, int nums[]);

void debug(int pos);

int main() {
    num_threads = 4;
    nums_len = 12;
    nums = (int *) malloc(nums_len * sizeof(int));
    nums0 = (int *) malloc(nums_len * sizeof(int));
    nums[0] = 5;
    nums[1] = 3;
    nums[2] = 2;
    nums[3] = 4;
    nums[4] = 6;
    nums[5] = 8;
    nums[6] = 10;
    nums[7] = 9;
    nums[8] = 1;
    nums[9] = 0;
    nums[10] = 12;
    nums[11] = 11;

//  nums = xxx
    initializeArray(fTree, num_threads);
    initializeArray(prefixSums, num_threads);

    for (int i = 0; i < 128; i++) {
        pthread_mutex_init(&locks[i], NULL);
        pthread_cond_init(&conds[i], NULL);
    }
    quickSort(0, nums_len - 1, nums);
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

void singleQuickSort(int lh, int rh, int nums[]) {
    // recursion exit
    if (lh >= rh) return;
    // lh, rh both inclusive
    int tmpArrLen = rh - lh + 1;
    int tmpArr[tmpArrLen];
    int pivot = nums[lh];
    int mid1 = 0;
    int mid2 = 0;
    for (int i = lh + 1; i <= rh; i++) {
        // rearrange nums bigger than pivot
        if (nums[i] <= pivot) {
            tmpArr[mid1] = nums[i];
            mid1++;
        }
        // rearrange nums smaller than pivot
        else if (nums[i] > pivot) {
            tmpArr[tmpArrLen - 1 - mid2] = nums[i];
            mid2++;
        }
    }
    tmpArr[mid1] = pivot;
    for (int i = lh; i <= rh; i++) {
        nums[i] = tmpArr[i - lh];
    }
    singleQuickSort(lh, lh + mid1 - 1, nums);
    singleQuickSort(rh - mid2 + 1, rh, nums);
}

int getPrefixSum(int idx) {
    int res = 0;

    // Compute Exclusive Prefix Sum
    while (idx >= 0) {
        pthread_mutex_lock(&locks[idx]);
        while (fTree[idx] == -1) pthread_cond_wait(&conds[idx], &locks[idx]);
        pthread_mutex_unlock(&locks[idx]);
        res += fTree[idx];
        idx = (idx & (idx + 1)) - 1;
    }

    return res;
}

int getDependencySum(int idx) {
    // Update fTree (As C array in Fenwick Tree); Signal waiting threads
    int x = idx;
    int y = 1;
    int res = 0;

//    debug(129);
    while (x & 1 == 1) {
        int dependency_idx = idx ^ y;
        pthread_mutex_lock(&locks[dependency_idx]);
        while (fTree[dependency_idx] == -1) pthread_cond_wait(&conds[dependency_idx], &locks[dependency_idx]);
        pthread_mutex_unlock(&locks[dependency_idx]);
//        printf("idx ^ y: %d\n", idx ^ y);
        res += fTree[dependency_idx];
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

    int currentSum = 0;

    for (int i = lh; i <= rh; i++) {
        currentSum += (nums[i] > pivot ? 1 : 0);
    }
//    debug(158);
    fTree[idx] = getDependencySum(idx) + currentSum;
    pthread_cond_broadcast(&conds[idx]);

    prefixSums[idx] = getPrefixSum(idx);
//    debug(164);

//    int tmpArrLen = rh - lh + 1;
//    int tmpArr[tmpArrLen];
    int offsetR = idx == 0 ? 0 : prefixSums[idx] - currentSum;
    int offsetL = idx == 0 ? 0 : (lh + 1 - prefixSums[idx] - currentSum);
    for (int i = lh; i <= rh; i++) {
        if (nums[i] > pivot) {
            nums0[nums_len - 1 - offsetR] = nums[i];
            offsetR++;
        }
        else {
            nums0[offsetL++] = nums[i];
        }
    }

//    // lh, rh both inclusive

//    int pivot = nums[lh];
//    int mid1 = 0;
//    int mid2 = 0;
//    for (int i = lh + 1; i <= rh; i++) {
//        // rearrange nums bigger than pivot
//        if (nums[i] <= pivot) {
//            tmpArr[mid1] = nums[i];
//            mid1++;
//        }
//            // rearrange nums smaller than pivot
//        else if (nums[i] > pivot) {
//            tmpArr[tmpArrLen - 1 - mid2] = nums[i];
//            mid2++;
//        }
//    }
//    tmpArr[mid1] = pivot;
//    for (int i = lh; i <= rh; i++) {
//        nums[i] = tmpArr[i - lh];
//    }


}

void quickSort(int lh, int rh, int nums[]) {
    // recursion exit
    if (lh >= rh) return;
    // clear fTree and prefixSums array
    initializeArray(fTree, num_threads);
    initializeArray(prefixSums, num_threads);
    // lh, rh both inclusive
    int tmpArrLen = rh - lh + 1;
    int tmpArr[tmpArrLen];

    int n = tmpArrLen / num_threads;

    // initialize args for threads
    for (int i = 0; i < num_threads; i++) {
        args_arr[i].idx = i;
        args_arr[i].lh = i * n + 1;
        args_arr[i].rh = i == num_threads - 1 ? rh : args_arr[i].lh + n - 1;
        args_arr[i].pivot = nums[lh];
    }

    // create threads
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&tids[i], NULL, prefixSumInclusive, &args_arr[i]);
//        debug(180);
    }
//    debug(181);
    for (int i = 0; i < num_threads; ++i) {
        int ret = pthread_join(tids[i], NULL);
        if (ret != 0) {
            printf("pthread_join error: error_code = %d\n", ret);
        }
//        debug(189);
    }
    // rearrange pivot
    int mid = nums_len - 1 - prefixSums[num_threads - 1];
    nums[mid] = nums[lh];

//    printArray(prefixSums, 4);
    printArray(nums, 12);
    // Clear prefixSums and fTree
    // ????????
    quickSort(lh, mid - 1, nums);
    quickSort(mid + 1, rh, nums);
}

void testSingleQuickSort() {
    int nums[] = {9, 9, 8, 8, 8, 6, 7, 5, 2, 3, 4, 9};
    int nums_len = sizeof(nums) / sizeof(int);
    singleQuickSort(0, nums_len - 1, nums);
    printArray(nums, nums_len);
    fflush(stdout);
}

void debug(int pos) {
    printf("****************** Debugging log at line %d ******************\n", pos);
    fflush(stdout);
}


