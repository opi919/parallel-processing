#include <bits/stdc++.h>
#include <mpi.h>

using namespace std;

// Function to send a string to a specific MPI process
void send_string(string text, int receiver)
{
    int length = text.size() + 1;
    MPI_Send(&length, 1, MPI_INT, receiver, 1, MPI_COMM_WORLD);
    MPI_Send(&text[0], length, MPI_CHAR, receiver, 1, MPI_COMM_WORLD);
}

// Function to receive a string from a specific MPI process
string receive_string(int sender)
{
    int length;
    MPI_Recv(&length, 1, MPI_INT, sender, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char *text = new char[length];
    MPI_Recv(text, length, MPI_CHAR, sender, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return string(text);
}

// Function to convert a segment of a vector to a string
string vector_to_string(vector<string> &words, int start, int end)
{
    string text = "";
    for (int i = start; i < min(static_cast<int>(words.size()), end); i++)
    {
        text += words[i] + "\n";
    }
    return text;
}

// Function to convert a string to a vector of words
vector<string> string_to_vector(string text)
{
    stringstream x(text);
    vector<string> words;
    string word;
    while (x >> word)
    {
        words.push_back(word);
    }
    return words;
}

// Function to check if a name matches and print the result
void check(string name, string phone, string search_name, int rank)
{
    if (name.size() != search_name.size())
    {
        return;
    }
    for (int i = 0; i < search_name.size(); i++)
    {
        if (name[i] != search_name[i])
        {
            return;
        }
    }
    printf("%s %s found by process %d.\n", name.c_str(), phone.c_str(), rank);
}

// Function to read phonebook data from files
void read_phonebook(vector<string> &file_names, vector<string> &names, vector<string> &phone_numbers)
{
    for (auto file_name : file_names)
    {
        ifstream file(file_name);
        string name, number;
        while (file >> name >> number)
        {
            names.push_back(name);
            phone_numbers.push_back(number);
        }
        file.close();
    }
}

int main(int argc, char **argv)
{
    // MPI Initialization
    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double start_time = MPI_Wtime();

    if (!rank)
    { // For the process with rank 0
        vector<string> names, phone_numbers;
        vector<string> file_names(argv + 1, argv + argc);
        read_phonebook(file_names, names, phone_numbers);
        int segment = names.size() / size + 1;

        // Sending segments of data to other processes
        for (int i = 1; i < size; i++)
        {
            int start = i * segment, end = start + segment;
            string names_to_send = vector_to_string(names, start, end);
            send_string(names_to_send, i);
            string phone_numbers_to_send = vector_to_string(phone_numbers, start, end);
            send_string(phone_numbers_to_send, i);
        }

        // Searching for a specific name in the first segment
        string name_to_search = "Bob";
        for (int i = 0; i < segment; i++)
        {
            check(names[i], phone_numbers[i], name_to_search, rank);
        }
    }
    else
    { // For processes with rank greater than 0
        // Receiving data segment from process 0
        string received_names = receive_string(0);
        vector<string> names = string_to_vector(received_names);
        string received_phone_numbers = receive_string(0);
        vector<string> phone_numbers = string_to_vector(received_phone_numbers);

        // Searching for a specific name in the received segment
        string name_to_search = "John";
        for (int i = 0; i < names.size(); i++)
        {
            check(names[i], phone_numbers[i], name_to_search, rank);
        }
    }

    double finish_time = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);

    // print individual process timings
    // printf("Process %d took %f seconds.\n", rank, finish_time - start_time);

    MPI_Finalize();

    return 0;
}
