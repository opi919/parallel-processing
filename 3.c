#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_NAME_LENGTH 100
#define MAX_PHONE_LENGTH 15

typedef struct
{
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
} Contact;

void readPhonebook(Contact *phonebook, int *numContacts, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Count the number of contacts in the file
    *numContacts = 0;
    while (!feof(file))
    {
        char line[MAX_NAME_LENGTH + MAX_PHONE_LENGTH + 2];
        if (fgets(line, sizeof(line), file) != NULL)
        {
            (*numContacts)++;
        }
    }

    // Reset the file pointer to the beginning
    fseek(file, 0, SEEK_SET);

    // Read contact information
    for (int i = 0; i < *numContacts; i++)
    {
        fscanf(file, "%s %s", phonebook[i].name, phonebook[i].phone);
    }

    fclose(file);
}

void searchContacts(Contact *phonebook, int numContacts, const char *searchName, int *matches, char ***matchingNumbers)
{
    *matches = 0;
    for (int i = 0; i < numContacts; i++)
    {
        if (strcmp(phonebook[i].name, searchName) == 0)
        {
            (*matches)++;
        }
    }

    *matchingNumbers = (char **)malloc((*matches) * sizeof(char *));
    int matchIndex = 0;

    for (int i = 0; i < numContacts; i++)
    {
        if (strcmp(phonebook[i].name, searchName) == 0)
        {
            (*matchingNumbers)[matchIndex] = strdup(phonebook[i].phone);
            matchIndex++;
        }
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2)
    {
        if (rank == 0)
        {
            fprintf(stderr, "Usage: %s <phonebook_file>\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char *phonebookFile = argv[1];
    Contact *phonebook;
    int numContacts;

    if (rank == 0)
    {
        // Allocate memory for phonebook
        numContacts = 100; // Assuming a maximum of 100 contacts
        phonebook = (Contact *)malloc(numContacts * sizeof(Contact));

        // Read phonebook
        readPhonebook(phonebook, &numContacts, phonebookFile);
    }

    MPI_Bcast(&numContacts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        phonebook = (Contact *)malloc(numContacts * sizeof(Contact));
    }

    MPI_Bcast(phonebook, numContacts * sizeof(Contact), MPI_BYTE, 0, MPI_COMM_WORLD);

    char searchName[MAX_NAME_LENGTH];
    if (rank == 0)
    {
        printf("Enter the name to search: ");
        fflush(stdout);
        scanf("%s", searchName);
    }

    MPI_Bcast(searchName, MAX_NAME_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double startTime = MPI_Wtime();

    int matches;
    char **matchingNumbers;
    searchContacts(phonebook, numContacts, searchName, &matches, &matchingNumbers);

    MPI_Barrier(MPI_COMM_WORLD);
    double endTime = MPI_Wtime();

    if (rank == 0)
    {
        printf("Total time: %f seconds\n", endTime - startTime);

        if (matches == 0)
        {
            printf("No matching contacts found.\n");
        }
        else
        {
            printf("Matching contacts and numbers:\n");
            for (int i = 0; i < matches; i++)
            {
                char *remainingName = strstr(phonebook[i].name, searchName);
                if (remainingName != NULL)
                {
                    // Skip the searched name and print the remaining part of the name
                    remainingName += strlen(searchName);
                    printf("%s%s: %s\n", phonebook[i].name, remainingName, matchingNumbers[i]);
                }
            }
        }
    }

    MPI_Finalize();
    return 0;
}
