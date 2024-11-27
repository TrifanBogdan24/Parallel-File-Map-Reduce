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
using Reducer = map<string, set<int>>;

struct MapperElement {
    string word;
    int fileID;        

    MapperElement(const string& wordValue, int fileID_Value)
        : word(wordValue), fileID(fileID_Value) {}
};

using Mapper = vector<MapperElement>;



enum ThreadPurpose {
    MAPPER_THREAD,
    REDUCER_THREAD,    
};

class ThreadArgument {
 public:
    int thread_ID;
    ThreadPurpose threadPurpose;
    
    // Pointeri la variabile
    pthread_barrier_t* barrier;


    int *numCompletedMappers;
    pthread_mutex_t* mutexNumCompletedMappers;

    // variabila conditionala
    pthread_cond_t* condCompletedMappers;

    int numMappers;
    int numReducers;




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
    int mapper_ID;
    int numInputFiles;
    vector<pthread_mutex_t>* mutexesMapperInputFileNames;
    vector<bool>* isProcessedFile;
    vector<string>* inputFileNames;
    vector<pthread_mutex_t>* mutexesMapperResults;
    vector<Mapper>* mappers;


 public:
    // Default constructor
    MapperThreadArgument():
        ThreadArgument(),
        isProcessedFile(NULL),
        inputFileNames(NULL),
        mutexesMapperResults(NULL)
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


vector<string> read_inptut_file(string inputFileName)
{
    ifstream fin(inputFileName);

    vector<string> mapperInputFileNames;

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
        mapperInputFileNames.push_back(fileName);
    }

    fin.close();
    return mapperInputFileNames;
}





set<string> get_unique_words_in_file(string &inputFileName)
{
    ifstream fin(inputFileName);


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


    int numMappers = threadArgument->numMappers;
    int mapper_ID = threadArgument->mapper_ID;
    int numInputFiles = threadArgument->numInputFiles;

    // Pointeri la valori
    int *numCompletedMappers = threadArgument->numCompletedMappers;
    pthread_mutex_t* mutexNumCompletedMappers = threadArgument->mutexNumCompletedMappers;
    vector<pthread_mutex_t>* mutexesMapperInputFileNames = threadArgument->mutexesMapperInputFileNames;
    vector<bool>* isProcessedFile = threadArgument->isProcessedFile;
    vector<string>* inputFileNames = threadArgument->inputFileNames;
    vector<pthread_mutex_t>* mutexesMapperResults = threadArgument->mutexesMapperResults;
    vector<Mapper>* mappers = threadArgument->mappers;

    pthread_cond_t* condCompletedMappers = threadArgument->condCompletedMappers;



    for (int i = 0; i < numInputFiles; i++) {
        // Thread-ul va lua un fisier care nu a fost citit deja, il marcheaza ca fiind procesat, si il citeste


        pthread_mutex_lock(&mutexesMapperInputFileNames->at(i));
        bool isFileToProcess = false;
        if (isProcessedFile->at(i) == false) {
            // Daca gasesc ca un fisier nu a fost procesat, il marchez si il citesc
            isFileToProcess = true;
            isProcessedFile->at(i) = true;
        }
        pthread_mutex_unlock(&mutexesMapperInputFileNames->at(i));

        if (!isFileToProcess) {
            continue;
        }

        // // Citeste continutul fisierului
        string inputFileName = inputFileNames->at(i);
        set<string> uniqueWords = get_unique_words_in_file(inputFileName);

        
        for (set<string>::iterator itr = uniqueWords.begin(); itr != uniqueWords.end(); itr++) {
            string word = *itr;
            int file_ID = i;
            
            mappers->at(mapper_ID).push_back(MapperElement(word, file_ID));
        }

    }



    pthread_mutex_lock(mutexNumCompletedMappers);
    (*numCompletedMappers) += 1;
    if ((*numCompletedMappers) == numMappers) {
        pthread_cond_broadcast(condCompletedMappers);
    }

    pthread_mutex_unlock(mutexNumCompletedMappers);


    delete threadArgument;
    pthread_exit(NULL);
}

// Functia de operatie Reducer care se va executa in paralel
void* thread_reducer_function(void *arg)
{
    ThreadArgument* threadArgument = (ThreadArgument*) arg;


    int numMappers = threadArgument->numMappers;
    int *numCompletedMappers = threadArgument->numCompletedMappers;
    pthread_mutex_t* mutexNumCompletedMappers = threadArgument->mutexNumCompletedMappers;

    pthread_cond_t* condCompletedMappers = threadArgument->condCompletedMappers;



    pthread_mutex_lock(mutexNumCompletedMappers);


    while ((*numCompletedMappers) != numMappers) {
        pthread_cond_wait(condCompletedMappers, mutexNumCompletedMappers);
    }

    pthread_mutex_unlock(mutexNumCompletedMappers);



    delete threadArgument;
    pthread_exit(NULL);
}





void printMapResults(vector<Mapper> &mapperResults)
{
    for (int i = 0; i < mapperResults.size(); i++) {
        cout << "Mapper " << i << ":\n";
        for (MapperElement &pair : mapperResults[i]) {
            cout << "{ " << pair.word << ", " << pair.fileID << " }\n";
        }
    }
}




int main(int argc, char* argv[])
{
   validate_input(argc, argv);

    int numMappers = chars_to_int(argv[1]);
    int numReducers = chars_to_int(argv[2]);
    string inputFileName = argv[3];

    vector<string> mapperInputFileNames = read_inptut_file(inputFileName);

    int numThreads = numMappers + numReducers;
    int numMapperInputFiles = mapperInputFileNames.size();
    
    vector<pthread_t> threads;
    vector<pthread_mutex_t> mutexesMapperInputFileNames;
    pthread_mutex_t mutexWordList;
    pthread_barrier_t barrier;

    vector<pthread_mutex_t> mutexesMapperResults;

    pthread_cond_t condCompletedMappers;
    pthread_cond_init(&condCompletedMappers, NULL);

    


    threads.resize(numThreads);

    mutexesMapperInputFileNames.resize(numMapperInputFiles);

    for (int i = 0; i < numMapperInputFiles; i++) {
        pthread_mutex_init(&mutexesMapperInputFileNames[i], NULL);
    }
    pthread_mutex_init(&mutexWordList, NULL);
    // Avem mapperInputFileNames.size() = numMapperFiles de operatii Mapper
    pthread_barrier_init(&barrier, NULL, numMapperInputFiles);

    vector<bool> isProcessedMapperFile;
    for (int i = 0; i < numMapperInputFiles; i++) {
        isProcessedMapperFile.push_back(false);
    }


    vector<Mapper> mappers;
    mappers.resize(numMappers);

    mutexesMapperResults.resize(numMappers);
    for (int i = 0; i < numMappers; i++) {
        pthread_mutex_init(&mutexesMapperResults[i], NULL);
    }

    int ret_code = 0;

    int numCompletedMappers = 0;
    pthread_mutex_t mutexNumCompletedMappers;
    pthread_mutex_init(&mutexNumCompletedMappers, NULL);

    for (int i = 0; i < numThreads; i++) {
        // Dowcasting is slow...I'm not using it
        ret_code = 0;

        if (i < numMappers) {
            MapperThreadArgument* threadArgument = new MapperThreadArgument();
            threadArgument->numMappers = numMappers;
            threadArgument->numReducers = numReducers;
            threadArgument->thread_ID = i;
            threadArgument->mapper_ID = i;
            threadArgument->numCompletedMappers = &numCompletedMappers;
            threadArgument->threadPurpose = MAPPER_THREAD;
            threadArgument->barrier = &barrier;
            threadArgument->numInputFiles = numMapperInputFiles; 
            threadArgument->mutexesMapperInputFileNames = &mutexesMapperInputFileNames;
            threadArgument->isProcessedFile = &isProcessedMapperFile;
            threadArgument->inputFileNames = &mapperInputFileNames;
            threadArgument->mutexesMapperResults = &mutexesMapperResults;
            threadArgument->mappers = &mappers;
            threadArgument->condCompletedMappers = &condCompletedMappers;
            threadArgument->mutexNumCompletedMappers = &mutexNumCompletedMappers;
            ret_code = pthread_create(&threads[i], NULL, thread_mapper_function, (void*) threadArgument);
        } else {
            ThreadArgument* threadArgument = new ThreadArgument();
            threadArgument->numMappers = numMappers;
            threadArgument->numReducers = numReducers;
            threadArgument->thread_ID = i;
            threadArgument->threadPurpose = REDUCER_THREAD;
            threadArgument->numCompletedMappers = &numCompletedMappers;
            threadArgument->mutexNumCompletedMappers = &mutexNumCompletedMappers;
            threadArgument->condCompletedMappers = &condCompletedMappers;


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


    // for debugging: printMapResults(mappers);




    pthread_cond_destroy(&condCompletedMappers);

    for (int i = 0; i < numMapperInputFiles; i++) {
        pthread_mutex_destroy(&mutexesMapperInputFileNames[i]);
    }

    pthread_mutex_destroy(&mutexWordList);
    for (int i = 0; i < numMappers; i++) {
        pthread_mutex_destroy(&mutexesMapperResults[i]);
    }
    pthread_mutex_destroy(&mutexNumCompletedMappers);
    return 0;
}

