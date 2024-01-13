#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

//mpirun -np 1 filename input.txt

#define MAX_WORD_LEN 100
#define MAX_WORDS 10000

typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordCount;

int compareWordCounts(const void *a, const void *b) {
    return ((WordCount *)b)->count - ((WordCount *)a)->count;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <filename>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    char filename[100];
    strcpy(filename, argv[1]);

    WordCount localWordCounts[MAX_WORDS];
    int localWordCount = 0;

    MPI_File file;
    MPI_Offset fileSize;

    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);

    MPI_File_get_size(file, &fileSize);

    MPI_Offset blockSize = fileSize / size;
    MPI_Offset start = rank * blockSize;
    MPI_Offset end = (rank == size - 1) ? fileSize : (rank + 1) * blockSize;

    MPI_File_set_view(file, start, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);

    char buffer[blockSize + MAX_WORD_LEN];
    MPI_File_read(file, buffer, blockSize, MPI_CHAR, MPI_STATUS_IGNORE);

    MPI_File_close(&file);

    // Tokenize the block and count words
    char *token = strtok(buffer, " \t\n");
    while (token != NULL) {
        int i;
        for (i = 0; i < localWordCount; i++) {
            if (strcmp(localWordCounts[i].word, token) == 0) {
                localWordCounts[i].count++;
                break;
            }
        }

        if (i == localWordCount) {
            strcpy(localWordCounts[localWordCount].word, token);
            localWordCounts[localWordCount].count = 1;
            localWordCount++;
        }

        token = strtok(NULL, " \t\n");
    }

    // Broadcast local word counts and total count to all processes
    MPI_Bcast(&localWordCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(localWordCounts, localWordCount * sizeof(WordCount), MPI_BYTE, 0, MPI_COMM_WORLD);

    // Perform necessary calculations on each process (e.g., counting occurrences)
    // No additional logic needed here

    // Gather and merge results from all processes
    WordCount globalWordCounts[MAX_WORDS * size];
    MPI_Gather(localWordCounts, localWordCount * sizeof(WordCount), MPI_BYTE,
               globalWordCounts, localWordCount * sizeof(WordCount), MPI_BYTE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Combine and sort results on master process
        int totalWordCount = localWordCount * size;
        qsort(globalWordCounts, totalWordCount, sizeof(WordCount), compareWordCounts);

        // Print sorted occurrences
        for (int i = 0; i < totalWordCount; i++) {
            printf("%s: %d\n", globalWordCounts[i].word, globalWordCounts[i].count);
        }
    }

    MPI_Finalize();
    return 0;
}
