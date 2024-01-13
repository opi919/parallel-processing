#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv); // Initialize MPI environment
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get the total number of processes

    printf("Hello from process %d of %d\n", rank, size);

    MPI_Finalize(); // Finalize MPI environment
    return 0;
}
//mpicc -o hello_mpi hello_mpi.c
//mpirun -np 4 hello_mpi
//mpirun -np 50 -use-hwthread-cpus --oversubscribe hello_mpil