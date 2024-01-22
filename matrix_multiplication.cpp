#include <iostream>
#include <vector>
#include <mpi.h>

using namespace std;

// Function to multiply two matrices A and B and store the result in C
void multiplyMatrices(const vector<vector<int>> &A, const vector<vector<int>> &B, vector<vector<int>> &C)
{
    int M = A.size();
    int N = A[0].size();
    int P = B[0].size();

    for (int i = 0; i < M; ++i)
    {
        for (int j = 0; j < P; ++j)
        {
            C[i][j] = 0;
            for (int k = 0; k < N; ++k)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int K, M, N, P;

    if (rank == 0)
    {
        K = atoi(argv[1]);
        M = atoi(argv[2]);
        N = atoi(argv[3]);
        P = atoi(argv[4]);
    }

    double start_time = MPI_Wtime();

    // Broadcast dimensions to all processes
    MPI_Bcast(&K, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&P, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process computes a subset of matrices
    int local_matrices = K / size;
    int remainder = K % size;

    int start = rank * local_matrices + min(rank, remainder);
    int end = start + local_matrices + (rank < remainder ? 1 : 0);

    // matrices A, B, and C with random values for each process
    for (int k = start; k < end; ++k)
    {
        vector<vector<int>> A(M, vector<int>(N));
        vector<vector<int>> B(N, vector<int>(P));
        vector<vector<int>> C(M, vector<int>(P));

        // Generate random values for matrices A and B on each process
        srand(rank * K + k + time(NULL)); // Use a unique seed for each process

        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                A[i][j] = rand() % 100 + 1; // Random value between 1 and 100
            }
        }

        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < P; ++j)
            {
                B[i][j] = rand() % 100 + 1; // Random value between 1 and 100
            }
        }

        // matrix multiplication for each local matrix
        multiplyMatrices(A, B, C);

        // Output for each local matrix
        cout << "Process " << rank << " - Matrix C[" << k << "]:" << endl;
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < P; ++j)
            {
                cout << C[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    double end_time = MPI_Wtime();
    if (rank == 0)
    {
        printf("Time taken: %f sec\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}
