#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <mpi.h>

using namespace std;

void sendStringToProcess(const string &text, int receiver)
{
    int length = text.size() + 1;
    MPI_Send(&length, 1, MPI_INT, receiver, 1, MPI_COMM_WORLD);
    MPI_Send(text.data(), length, MPI_CHAR, receiver, 1, MPI_COMM_WORLD);
}

string receiveStringFromProcess(int sender)
{
    int length;
    MPI_Recv(&length, 1, MPI_INT, sender, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    string text(length, ' ');
    MPI_Recv(text.data(), length, MPI_CHAR, sender, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return text;
}

string convertVectorToString(const vector<string> &words, int start, int end)
{
    stringstream ss;
    for (int i = start; i < min(end, static_cast<int>(words.size())); i++)
    {
        ss << words[i] << "\n";
    }
    return ss.str();
}

vector<string> convertStringToVector(const string &text)
{
    stringstream ss(text);
    vector<string> words;
    string word;
    while (ss >> word)
    {
        words.push_back(word);
    }
    return words;
}

void searchAndPrint(const string &name, const string &phone, const string &searchName, int rank)
{
    if (name.size() == searchName.size() && equal(name.begin(), name.end(), searchName.begin()))
    {
        cout << name << " " << phone << " found by process " << rank << ".\n";
    }
}

void readPhonebook(const vector<string> &fileNames, vector<string> &names, vector<string> &phoneNumbers)
{
    for (const auto &fileName : fileNames)
    {
        ifstream file(fileName);
        string name, number;
        while (file >> name >> number)
        {
            names.push_back(name);
            phoneNumbers.push_back(number);
        }
        file.close();
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double startTime = MPI_Wtime();

    if (rank == 0)
    {
        vector<string> names, phoneNumbers;
        vector<string> fileNames(argv + 1, argv + argc);
        readPhonebook(fileNames, names, phoneNumbers);
        int segmentSize = names.size() / size + 1;

        for (int i = 1; i < size; i++)
        {
            int start = i * segmentSize, end = start + segmentSize;
            sendStringToProcess(convertVectorToString(names, start, end), i);
            sendStringToProcess(convertVectorToString(phoneNumbers, start, end), i);
        }
    }
    else
    {
        string receivedNames = receiveStringFromProcess(0);
        string receivedPhoneNumbers = receiveStringFromProcess(0);
        vector<string> names = convertStringToVector(receivedNames);
        vector<string> phoneNumbers = convertStringToVector(receivedPhoneNumbers);

        string nameToSearch = "John";
        for (size_t i = 0; i < names.size(); i++)
        {
            searchAndPrint(names[i], phoneNumbers[i], nameToSearch, rank);
        }
    }

    double finishTime = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);

    // print individual process timings
    // cout << "Process " << rank << " took " << finishTime - startTime << " seconds.\n";

    MPI_Finalize();

    return 0;
}
