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
#include <cstring>
#include <cctype>

using namespace std;

// aliases for data types
using Mapper = map<string, int>;
using Reducer = map<string, set<int>>;

struct MapperElement {
    string word;
    int fileID;
};


enum ThreadPurpose {
    MAPPER_THREAD,
    REDUCER_THREAD,    
};

class ThreadArgument {
 public:
    int thread_ID;
    ThreadPurpose threadPurpose;
    
    // Pointeri la variabile
    pthread_mutex_t* mutexWordList;
    pthread_barrier_t* barrier;



 public:
    // Default constructor
    ThreadArgument()
        : thread_ID(-1), threadPurpose(MAPPER_THREAD)
    {
    }


    // Desctructor
    ~ThreadArgument()
    {
    }

};


class MapperThreadArgument : public ThreadArgument {
 public:
    int numFiles;
    pthread_mutex_t* mutexFileList;
    vector<bool>* isProcessedFile;
    vector<string>* fileNames;

 public:
    // Default constructor
    MapperThreadArgument():
        ThreadArgument(),
        mutexFileList(NULL),
        isProcessedFile(NULL),
        fileNames(NULL)
    {
    }

    // Destructor
    ~MapperThreadArgument()
    {
    }
};



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





set<string> get_unique_words_in_file(string &inputFileName)
{
    ifstream fin(inputFileName);

    cout << inputFileName << "\n";

    if (!fin.is_open()) {
        cerr << "[ERROR] Cannot open input file <" << inputFileName << ">\n";
        // Return an empty set
        return set<string>();
    }

    set<string> uniqueWords;


    string line;

    while (getline(fin, line)) {
        for (char& chr : line) {
            chr = isalpha(chr) ? tolower(chr) : ' ';
        }

        char* str = strtok(&line[0], " \t\n");

        while (str != NULL) {
            string word(str);

            if (!word.empty()) {
                uniqueWords.insert(word);
            }

            str = strtok(NULL, " \t\n");
        }

    }


    fin.close();
    return uniqueWords;
}



// Functia de operatie Mapper care se va executa in paralel
void* thread_mapper_function(void *arg)
{
    MapperThreadArgument* threadArgument = (MapperThreadArgument*) arg;


    for (int i = 0; i < threadArgument->numFiles; i++) {
        // Thread-ul va lua un fisier care nu a fost citit deja, il marcheaza ca fiind procesat, si il citeste

        pthread_mutex_lock(threadArgument->mutexFileList);
        bool isFileToProcess = false;
        if (threadArgument->isProcessedFile->at(i) == false) {
            // Daca gasesc ca un fisier nu a fost procesat, il marchez si il citesc
            isFileToProcess = true;
            threadArgument->isProcessedFile->at(i) = true;
        }
        pthread_mutex_unlock(threadArgument->mutexFileList);

        if (!isFileToProcess) {
            continue;
        }

        // // Citeste continutul fisierului
        string fileName = threadArgument->fileNames->at(i);
        set<string> uniqueWords = get_unique_words_in_file(fileName);

        // TODO: merge with mapper result
    }




    delete threadArgument;
    pthread_exit(NULL);
}

// Functia de operatie Reducer care se va executa in paralel
void* thread_reducer_function(void *arg)
{
    ThreadArgument* threadArgument = (ThreadArgument*) arg;
    delete threadArgument;
    pthread_exit(NULL);
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
    pthread_mutex_t mutexMapperFileNames;
    pthread_mutex_t mutexWordList;
    // Folosesc o bariera pentru a impune ca mai intai Maparile sa fie executate inaintea operatiilor Reduce
    // Avem mapperFileNames.size() mapari
    pthread_barrier_t barrier;

    // cate un mutex pentru fiecare mapper
    pthread_mutex_t* mutexMapper;

    
    int numThreads = numMappers + numReducers;
    int numMapperFiles = mapperFileNames.size();
    

    threads = (pthread_t *) malloc(numThreads * sizeof(pthread_t));
    

    pthread_mutex_init(&mutexMapperFileNames, NULL);
    pthread_mutex_init(&mutexWordList, NULL);
    // Avem mapperFileNames.size() = numMapperFiles de operatii Mapper
    pthread_barrier_init(&barrier, NULL, numMapperFiles);

    vector<bool> isProcessedMapperFile;
    for (int i = 0; i < numMapperFiles; i++) {
        isProcessedMapperFile.push_back(false);
    }

    vector<MapperElement> mappers;
    for (int i = 0; i < numMappers; i++) {
        pthread_mutex_init(&mutexMapper[i], NULL);
    }


    int ret_code = 0;


    for (int i = 0; i < numThreads; i++) {
        // Dowcasting is slow...I'm not using it
        ret_code = 0;

        if (i < numMappers) {
            MapperThreadArgument* threadArgument = new MapperThreadArgument();
            threadArgument->thread_ID = i;
            threadArgument->threadPurpose = MAPPER_THREAD;
            threadArgument->mutexWordList = &mutexWordList;
            threadArgument->barrier = &barrier;
            threadArgument->numFiles = numMapperFiles; 
            threadArgument->mutexFileList = &mutexMapperFileNames;
            threadArgument->isProcessedFile = &isProcessedMapperFile;
            threadArgument->fileNames = &mapperFileNames;

    
            ret_code = pthread_create(&threads[i], NULL, thread_mapper_function, (void*) threadArgument);
        } else {
            ThreadArgument* threadArgument = new ThreadArgument();
            threadArgument->thread_ID = i;
            threadArgument->threadPurpose = REDUCER_THREAD;
            ret_code = pthread_create(&threads[i], NULL, thread_reducer_function, (void*) threadArgument);
        }

        if (ret_code) {
            cerr << "[ERROR] Eroare la crearea thread-ului: " << i << "\n";
        }
    }

    void *status;
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], &status);
        
        if (ret_code) {
            cerr << "[ERROR] Eroare la terminarea thread-ului: " << i << "\n";
        }
    }





    free(threads);
    pthread_mutex_destroy(&mutexMapperFileNames);
    pthread_mutex_destroy(&mutexWordList);
    for (int i = 0; i < numMappers; i++) {
        pthread_mutexattr_destroy(mutexMapper[i]);
    }
    return 0;
}

