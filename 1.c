#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

//mpirun -np 4 filename K M N P

#define M 16
#define N 16
#define P 16

void matrix_multiply(int A[M][N], int B[N][P], int result[M][P], int m, int n, int p, int start_row, int end_row) {
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < p; j++) {
            result[i][j] = 0;
            for (int k = 0; k < n; k++) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 5) {
        if (rank == 0) {
            printf("Usage: %s K M N P\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int K = atoi(argv[1]);
    int m = atoi(argv[2]);
    int n = atoi(argv[3]);
    int p = atoi(argv[4]);

    int A[M][N], B[N][P], result[M][P];

    // Initialize matrices A and B (you need to provide your own initialization logic)

    // Distribute data among processes
    int rows_per_process = m / size;
    int start_row = rank * rows_per_process;
    int end_row = (rank + 1) * rows_per_process;

    // Perform matrix multiplication
    double start_time = MPI_Wtime();
    for (int k = 0; k < K; k++) {
        matrix_multiply(A, B, result, m, n, p, start_row, end_row);
    }
    double end_time = MPI_Wtime();

    // Gather results from all processes
    MPI_Gather(result + start_row, rows_per_process * P, MPI_INT, result, rows_per_process * P, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate and print the time taken
    if (rank == 0) {
        printf("Time taken for multiplication: %f seconds\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}
