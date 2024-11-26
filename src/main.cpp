// C++ libraries
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <stdexcept>

// C libraries
#include <pthread.h>
#include <stdlib.h>

using namespace std;

// aliases for data types
using Mapper = map<string, int>;
using Reducer = map<string, set<int>>;


void thread_function(void *arg)
{
    pthread_exit(NULL);
}


int chars_to_int(char *str)
{
    if (str == NULL) {
        return -1;
    }

    int num = 0;

    for (char *ptr = str; *ptr != '\0'; ptr++) {
        if (*ptr < '0' || *ptr > '9') {
            return 1;
        }
        num = num * 10 + (*ptr - '0');
    }

    return num;
}

void validate_input(int argc, char* argv[])
{
    if (argc != 4) {
        cerr << "Invalid number of arguments!\n";
        cerr << "The program expects exactly 3 arguments in the command line!\n";
        cerr << "./exe <numar_mapperi> <numar_reduceri> <fisier_intrare>\n";
        exit(EXIT_FAILURE);
    }


    bool isValidInput = true;

    int numMappers = chars_to_int(argv[1]);
    int numReducers = chars_to_int(argv[2]);


    if (numMappers <= 0) {
        cerr << "[ERROR] The frist argument in invalid!\n";
        cerr << "The first argument (number of mappers) is expected to be a positive non-zero number.\n";
        isValidInput = false;
    }

    if (numReducers <= 0) {
        cerr << "[ERROR] The frist argument in invalid!\n";
        cerr << "The first argument (number of mappers) is expected to be a positive non-zero number.\n";
        isValidInput = false;
    }


    if (!isValidInput) {
        exit(EXIT_FAILURE);
    }
}


// functia citeste dintr-un fisier N nume de fisiere (pe care le returneaza in parametrul dat prin referinta)
void read_inptut_file(string inputFileName, vector<string> &mapperFileNames)
{
    ifstream fin(inputFileName);

    if (!fin.is_open()) {
        cerr << "[ERROR] Cannot open input file <" << inputFileName << ">\n";
        exit(EXIT_FAILURE);
    }


    int numFileNames = 0;
    fin >> numFileNames;
    // Reading the new-line ('\n') character
    fin.get();

    string fileName;

    for (int i = 0; i < numFileNames; i++) {
        getline(fin, fileName);
        mapperFileNames.push_back(fileName);
    }

    fin.close();
}


void initialize(int numThreads, int numMapperFiles,
    pthread_t* &threads, vector<int> &thread_ID,
    pthread_mutex_t &mutexWordList, pthread_mutex_t* &mutexMapperFiles, pthread_barrier_t barrier)
{
    threads = (pthread_t *) malloc(numThreads * sizeof(pthread_t));
    
    vector<int> thread_id;
    for (int i = 0; i < numThreads; i++) {
        thread_ID.push_back(i);
    }

    mutexMapperFiles = (pthread_mutex_t *) malloc(numMapperFiles * sizeof(pthread_mutex_t));
    for (int i = 0; i < numMapperFiles; i++) {
        pthread_mutex_init(mutexMapperFiles, NULL);
    }

    pthread_mutex_init(&mutexWordList, NULL);

    // Avem mapperFileNames.size() = numMapperFiles de operatii Mapper
    pthread_barrier_init(&barrier, NULL, numMapperFiles);
}


void dealocate(int numThreads, int numMapperFiles,
    pthread_t* &threads, vector<int> &thread_ID,
    pthread_mutex_t &mutexWordList, pthread_mutex_t* &mutexMapperFiles, pthread_barrier_t barrier)
{
    free(threads);

    pthread_mutex_destroy(&mutexWordList);
    for (int i = 0; i < numMapperFiles; i++) {
        pthread_mutex_destroy(&mutexMapperFiles[i]);
    }
    free(mutexMapperFiles);
}

int main(int argc, char* argv[])
{
   validate_input(argc, argv);

    int numMappers = chars_to_int(argv[1]);
    int numReducers = chars_to_int(argv[2]);
    string inputFileName = argv[3];

    vector<string> mapperFileNames;

    read_inptut_file(inputFileName, mapperFileNames);

    pthread_t *threads;
    vector<int> thread_ID;
    pthread_mutex_t *mutexMapperFiles;
    pthread_mutex_t mutexWordList;
    // Folosesc o bariera pentru a impune ca mai intai Maparile sa fie executate inaintea operatiilor Reduce
    // Avem mapperFileNames.size() mapari
    pthread_barrier_t barrier;

    initialize(numMappers + numReducers, mapperFileNames.size(),
        threads, thread_ID, mutexWordList, mutexMapperFiles, barrier);

    dealocate(numMappers + numReducers, mapperFileNames.size(),
        threads, thread_ID, mutexWordList, mutexMapperFiles, barrier);


    return 0;
}

