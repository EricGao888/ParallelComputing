// Each thread sort its fragment
// Each fragment remains sorted, every time do a merge
// Find the pivot using medium of medium
// Pair the thread in a curricular way (try using subtraction)
//  memcpy(array + displs[src], buffer, sendCounts[src]);

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
int *subarray;
int subarrayLen;

int main(int argc, char *argv[]) {
    srand(time(0));
    struct timeval start, end;

    int *inputArray, *ansArray;
    int inputArrayLen;

    inputArrayLen = atoi(argv[1]);

    MPInitialize(argc, argv);

    if (size & (size - 1) != 0) {
        if (rank == 0) printf("Number of threads not Exp of 2! Program terminates!\n");
        exit(1);
    }

    if (rank == 0) {
        printf("Preparing data...\n");
        fflush(stdout);
        inputArray = (int *) malloc(inputArrayLen * sizeof(int));
        ansArray = (int *) malloc(inputArrayLen * sizeof(int));

        for (int i = 0; i < inputArrayLen; i++) {
            inputArray[i] = rand() % inputArrayLen;
            ansArray[i] = inputArray[i];
        }
    }


//    preprocess(inputArray, inputArrayLen);
    distribute(inputArray, inputArrayLen);
    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&start, NULL);
    quickSort(inputArray);
    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&end, NULL);
    if (rank == 0) printf("Time use for parallel version: %ld\n", (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    gather(inputArray);
    if (rank == 0) {
        gettimeofday(&start, NULL);
        qsort(ansArray, inputArrayLen, sizeof(int), cmpFunction);
        gettimeofday(&end, NULL);
        printf("Time use for qsort version: %ld\n", (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
        check(inputArray, ansArray, inputArrayLen);
    }
//    if (rank == 0) testBinarySearch();
//    getPivot(MPI_COMM_WORLD);
    // Finalize the MPI environment.
    MPI_Finalize();
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
}

void distribute(int *array, int len) {
    int displs[size];
    int sendCounts[size];
    int remain = len % size;
    int offset = 0;

    for (int i = 0; i < size; i++) {
        sendCounts[i] = len / size;
        if (remain > 0) {
            sendCounts[i]++;
            remain--;
        }
        displs[i] = offset;
        offset += sendCounts[i];
    }

    subarrayLen = sendCounts[rank];
    subarray = (int *) malloc(subarrayLen * sizeof(int));

    // divide the data among processes as described by sendcounts and displs
    MPI_Scatterv(array, sendCounts, displs, MPI_INT, subarray, subarrayLen, MPI_INT, 0, MPI_COMM_WORLD);
//    printf("Subarray from process rank %d: ", rank);
//    printArray(subarray, subarrayLen);
}

void gather(int *inputArray) {
    MPI_Status status;
    int *recvBuffer;
    int recvBufferLen;
    if (rank != 0) MPI_Send(subarray, subarrayLen, MPI_INT, 0, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        memcpy(inputArray, subarray, subarrayLen * sizeof(int));
        int offset = subarrayLen;
        for (int src = 1; src < size; src++) {
            MPI_Probe(src, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &recvBufferLen);
            MPI_Recv(inputArray + offset, recvBufferLen, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            offset += recvBufferLen;
        }
    }
}

int getPivot(MPI_Comm communicator, int commNum) {
    int localMedium;
    if (subarrayLen == 0) localMedium = -1;
    else localMedium = subarray[(subarrayLen & 1) ? (subarrayLen / 2) : (subarrayLen / 2 - 1)];
    int mediums[commNum];
    MPI_Allgather(&localMedium, 1, MPI_INT, &mediums, 1, MPI_INT, communicator);
    qsort(mediums, commNum, sizeof(int), cmpFunction);
    int globalMedium = mediums[(commNum & 1) ? (commNum / 2) : (commNum / 2 - 1)];
//    if (rank == 0) {
//        printArray(mediums, size);
//        printf("Pivot: %d\n", globalMedium);
//    }
    return globalMedium;
//    return 0;
}

void quickSort(int *inputArray) {
//    printf("rank %d, subarray len: %d\n", rank, subarrayLen);
    qsort(subarray, subarrayLen, sizeof(int), cmpFunction);
    MPI_Comm communicator = MPI_COMM_WORLD;
    MPI_Comm newCommunicator;
    MPI_Status status;
    int commNum = size;
    int *recvBuffer;
    int recvBufferLen;
    int round = -1;
    for (int i = size; i > 0; i >>= 1) round++;
//    debug(169, rank);
    for (int i = round; i > 0; i--) {
        int flip = 1;
        int tmp = rank;
        for (int j = 1; j < i; j++) {
            flip <<= 1;
            tmp >>= 1;
        }
        tmp &= 1;
        int dst = rank ^ flip;
        int pivot = getPivot(communicator, commNum);
        int idx = binarySearch(subarray, 0, subarrayLen - 1, pivot);
//        printf("rank %d, pivot idx: %d\n", rank, idx);
//        printf("rank: %d, dst: %d\n", rank, dst);
//        fflush(stdout);
        if (tmp == 0) {
            MPI_Send(subarray + idx, subarrayLen - idx, MPI_INT, dst, 0, MPI_COMM_WORLD);
//            printf("Send Done from rank %d\n", rank);
//            fflush(stdout);
            MPI_Probe(dst, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &recvBufferLen);
            recvBuffer = (int *)malloc(recvBufferLen * sizeof(int));
            MPI_Recv(recvBuffer, recvBufferLen, MPI_INT, dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//            printf("Receive Done from rank %d\n", rank);
//            fflush(stdout);
        }
        else {
            MPI_Probe(dst, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &recvBufferLen);
            recvBuffer = (int *)malloc(recvBufferLen * sizeof(int));
            MPI_Recv(recvBuffer, recvBufferLen, MPI_INT, dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//            printf("Receive Done from rank %d\n", rank);
//            fflush(stdout);
            MPI_Send(subarray, idx, MPI_INT, dst, 0, MPI_COMM_WORLD);
//            printf("Send Done from rank %d\n", rank);
//            fflush(stdout);
        }
//        debug(194, rank);
        if (tmp == 0) {
            subarray = merge(subarray, idx - 0, recvBuffer, recvBufferLen);
            MPI_Comm_split(communicator, 0, 0, &newCommunicator);
            subarrayLen = idx + recvBufferLen;
        }
        else {
            subarray = merge(subarray + idx, subarrayLen - idx, recvBuffer, recvBufferLen);
            MPI_Comm_split(communicator, 1, 0, &newCommunicator);
            subarrayLen = subarrayLen - idx + recvBufferLen;
        }
        communicator = newCommunicator;
        commNum /= 2;
        free(recvBuffer);
    }
}


