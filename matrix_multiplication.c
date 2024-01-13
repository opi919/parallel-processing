#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 500 // Size of the square matrices

void printMatrix(int mat[N][N], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%d\t", mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (N % size != 0) {
        if (rank == 0) {
            printf("The matrix size N must be evenly divisible by the number of processes.\n");
        }
        MPI_Finalize();
        return 1;
    }

    int A[N][N] = {
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10},
        {11, 12, 13, 14, 15},
        {16, 17, 18, 19, 20},
        {21, 22, 23, 24, 25}
    };

    int B[N][N] = {
        {1, 0, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 1}
    };

    int local_A[N / size][N]; // Each process holds N/size rows of matrix A
    int local_C[N / size][N]; // Each process computes N/size rows of the result matrix C

    MPI_Scatter(A, N * N / size, MPI_INT, local_A, N * N / size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < N / size; i++) {
        for (int j = 0; j < N; j++) {
            local_C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                local_C[i][j] += local_A[i][k] * B[k][j];
            }
        }
    }

    int C[N][N];
    MPI_Gather(local_C, N * N / size, MPI_INT, C, N * N / size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Matrix A:\n");
        printMatrix(A, N);

        printf("Matrix B:\n");
        printMatrix(B, N);

        printf("Matrix C (A x B):\n");
        printMatrix(C, N);
    }

    MPI_Finalize();
    return 0;
}
