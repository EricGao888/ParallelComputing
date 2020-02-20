// Each thread sort its fragment
// Each fragment remains sorted, every time do a merge
// Find the pivot using medium of medium
// Pair the thread in a curricular way (try using subtraction)

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>
#include "QuickSort.h"

int size; // total number of processes
int rank; // rank of the current process

int main(int argc, char *argv[]) {
    srand(time(0));
    struct timeval start, end;

    int *inputArray, *ansArray;
    int inputArrayLen;

    inputArrayLen = atoi(argv[1]);

    MPInitialize(argc, argv);

    if (rank == 0) {
        inputArray = (int *) malloc(inputArrayLen * sizeof(int));
        ansArray = (int *) malloc(inputArrayLen * sizeof(int));

        for (int i = 0; i < inputArrayLen; i++) {
            inputArray[i] = rand() % inputArrayLen;
            ansArray[i] = inputArray[i];
        }

        qsort(ansArray, inputArrayLen, sizeof(int), cmpFunction);
        check(inputArray, ansArray, inputArrayLen);
    }

//    MPI_Barrier(MPI_COMM_WORLD);

    preprocess(inputArray, inputArrayLen);

    // Finalize the MPI environment.
    MPI_Finalize();
}

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

void debug(int pos) {
    printf("****************** Debugging log at line %d ******************\n", pos);
    fflush(stdout);
}

void printArray(int array[], int len) {
    for (int i = 0; i < len; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
    fflush(stdout);
}

void quickSort(int *array, int lh, int rh, int preprocess) {
//    if (preprocess == 1)
}

void MPInitialize(int argc, char *argv[]) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Process %s created! Rank %d out of %d processes\n",
           processor_name, rank, size);
    fflush(stdout);
}

void preprocess(int *array, int len) {
    int displs[size];
    int sendCounts[size];
    int recvBuffer[len];
    int remain = len % size;
    int offset = 0;
    int buffer[len];

    for (int i = 0; i < size; i++) {
        sendCounts[i] = len / size;
        if (remain > 0) {
            sendCounts[i]++;
            remain--;
        }

        displs[i] = offset;
        offset += sendCounts[i];
    }

//    // print calculated send counts and displacements for each process
//    if (rank == 0) {
//        for (int i = 0; i < size; i++) {
//            printf("sendCounts[%d] = %d\tdispls[%d] = %d\n", i, sendCounts[i], i, displs[i]);
//        }
//    }

    // divide the data among processes as described by sendcounts and displs
    MPI_Scatterv(array, sendCounts, displs, MPI_INT, recvBuffer, len, MPI_INT, 0, MPI_COMM_WORLD);
    qsort(recvBuffer, sendCounts[rank], sizeof(int), cmpFunction);


    // change send and receive to gatherv
    for (int src = 0; src < size; src++) {
        if (rank == src) {
            MPI_Send(recvBuffer, sendCounts[src], MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0) {
        for (int src = 0; src < size; src++) {
            MPI_Recv(buffer, sendCounts[src], MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            memcpy(array + displs[src], buffer, sendCounts[src]);
        }
        printf("Array after preprocessing: ");
        printArray(array, len);
    }

//    // print what each process received
//    printf("rank %d: ", rank);
//    fflush(stdout);
//    for (int i = 0; i < sendCounts[rank]; i++) {
//        printf("%d ", recvBuffer[i]);
//        fflush(stdout);
//    }
//    printf("\n");
}