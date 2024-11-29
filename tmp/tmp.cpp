// C++ libraries
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>

// C libraries
#include <pthread.h>
#include <stdlib.h>
#include <cstring>
#include <cctype>
#include <unistd.h>

using namespace std;


struct MapperElement {
    string word;
    int fileID;        

    MapperElement(const string& wordValue, const int fileID_Value)
        : word(wordValue), fileID(fileID_Value) {}
};

using MapperResult = vector<MapperElement>;




struct WordListElement {
    string word;
    set<int> fileIDs;

    WordListElement(const string &wordValue) :
        word(wordValue)
    {
    }

    WordListElement(const string &wordValue, const set<int> &fileIDValues):
        word(wordValue), fileIDs(fileIDValues)
    {
    }


    ~WordListElement()
    {
    }
};

using WordList = vector<WordListElement>;




enum ThreadPurpose {
    MAPPER_THREAD,
    REDUCER_THREAD,
};

class ThreadArgument {
 public:
    int thread_ID;
    ThreadPurpose threadPurpose;
    
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
    vector<MapperResult>* mapperResults;
    pthread_mutex_t* mutexMapperResults;



 public:
    // Default constructor
    MapperThreadArgument():
        ThreadArgument(),
        isProcessedFile(NULL),
        inputFileNames(NULL)
    {
    }

    // Destructor
    ~MapperThreadArgument()
    {
    }
};



class ReducerThreadArgument : public ThreadArgument {
 public:
    int reducer_ID;
    int numInputFiles;
    vector<pthread_mutex_t>* mutexesMapperResults;
    vector<MapperResult>* mapperResults;

    vector<bool>* isProcessedMapperResults;
    vector<pthread_mutex_t>* mutexesProcessedMapperResults;

    pthread_mutex_t* mutexWordList;
    WordList* wordList;

    pthread_barrier_t* barrierComputeWordList;

    // pthread_mutex_t* mutexIsSplitWordList;
    // bool* isSplitWordList;

    // vector<pthread_mutex_t>* mutexesIsSplitWordListForLetter;
    // vector<bool>* isSplitWordListForLetter;

    // vector<WordListChunck>* wordListChunks;
    // vector<bool>* isPrintedWordListChunk;
    // vector<pthread_mutex_t>* mutexesIsPrintedWordListChunk;


 public:
    // Default constructor
    ReducerThreadArgument()
    {
    }

    // Destructor
    ~ReducerThreadArgument()
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
        cerr << "The first argument (number of mapperResults) is expected to be a positive non-zero number.\n";
        isValidInput = false;
    }

    if (numReducers <= 0) {
        cerr << "[ERROR] The frist argument in invalid!\n";
        cerr << "The first argument (number of mapperResults) is expected to be a positive non-zero number.\n";
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


pthread_mutex_t printmtx;


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
                pthread_mutex_lock(&printmtx);
                cout << word << "->" << inputFileName << "\n";
                uniqueWords.insert(word);
               pthread_mutex_unlock(&printmtx);

            }

            str = strtok(NULL, " \t\n");
        }

    }


    fin.close();
    return uniqueWords;
}


// ordinea alfabetica
int compare_words(string &word1, string &word2)
{
    if (word1 < word2) {
        // "-1" daca word1 inaintea lui word2 in ordinea lexicografica
        return -1;
    } else if (word1 > word2) {
        // "1" daca word1 dupa word2 in ordinea lexicografica
        return 1;
    }
    return 0;
}


void insert_in_word_list(WordList* wordList, MapperElement &mapperElement)
{
    string word = mapperElement.word;
    int fileID = mapperElement.fileID;

    int length = wordList->size();

    if (length == 0) {
        wordList->push_back(WordListElement(word, {fileID}));
        return;
    }

    if (word < wordList->at(0).word) {
        wordList->insert(wordList->begin(), WordListElement(word, {fileID}));
        return;
    }

    if (word > wordList->at(length - 1).word) {
        wordList->push_back(WordListElement(word, {fileID}));
        return;
    }


    int left = 0;
    int right = length - 1;
    int middle = 0;

    while (left <= right) {
        middle = left + (right - left) / 2;

        if (wordList->at(middle).word < word) {
            left = middle + 1;
        } else if (wordList->at(middle).word > word) {
            right = middle - 1;
        } else {
            // then: wordList->at(middle).word == word
            // Updating word's set
            wordList->at(middle).fileIDs.insert(fileID);
            return;
        }
    }

    wordList->insert(wordList->begin() + left, WordListElement(word, {fileID}));
}

struct WordListChunck {
    char ch;
    int firstIndex;
    int lastIndex;

    WordListChunck() {}

    WordListChunck(char ch_value, int firstIndex_value, int lastIndex_value):
        ch(ch_value), firstIndex(firstIndex_value), lastIndex(lastIndex_value)
    {
    }

    ~WordListChunck() {}
};

optional<WordListChunck> find_character_chunk_in_word_list(WordList* wordList, char ch)
{
    int firstIndex = -1;
    int lastIndex = -1;
    int left = 0;
    int right = 0;
    int middle = 0;
    

    // Gasirea indexului primului cuvant care incepe cu caracterul "ch"
    left = 0;
    right = wordList->size() - 1;

    while (left <= right) {
        middle = left + (right - left) / 2;
        
        if (wordList->at(middle).word[0] < ch) {
            left = middle + 1;
        } else if (wordList->at(middle).word[0] > ch) {
            right = middle + 1;
        } else {
            // then: wordList->at(middle).word[0] == ch)
            firstIndex = middle;
            right = middle -1;
        }
    }


    if (firstIndex == -1) {
        return nullopt;
    }


    // Gasirea indexului ultimului cuvant care incepe cu caracterul "ch"
    left = 0;
    right = wordList->size() - 1;

    while (left <= right) {
        middle = left + (right - left) / 2;

        if (wordList->at(middle).word[0] < ch) {
            left = middle + 1;
        } else if (wordList->at(middle).word[0] > ch) {
            right = middle - 1;
        } else {
            // then: wordList->at(middle).word[0] == ch
            lastIndex = middle;
            left = middle + 1;
        }
    }


    if (lastIndex == -1) {
        return nullopt;
    }

    return WordListChunck(ch, firstIndex, lastIndex);
}


void write_file_IDs(ofstream &fout, vector<int> &fileIDs)
{

    for (unsigned int i = 0; i < fileIDs.size(); i++) {
        fout << fileIDs[i];
        if (i != fileIDs.size() - 1) {
            fout << " ";
        }
    }
}

void write_word_file_IDs(ofstream &fout, WordList &wordList, unsigned int &idx)
{
    fout << wordList[idx].word << ": [";
    
    vector<int> fileIDs(wordList[idx].fileIDs.begin(), wordList[idx].fileIDs.end());
    sort(fileIDs.begin(), fileIDs.end());

    write_file_IDs(fout, fileIDs);
    fout << "]\n";
}


void write_word_list_chunk(WordList* wordList)
{
    // TODO: fol functiile de mai sus pt scriere
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

    pthread_mutex_t* mutexMapperResults = threadArgument->mutexMapperResults;
    vector<MapperResult>* mapperResults = threadArgument->mapperResults;

    pthread_cond_t* condCompletedMappers = threadArgument->condCompletedMappers;



    for (int i = 0; i < numInputFiles; i++) {
        // Thread-ul va lua un fisier care nu a fost citit deja, il marcheaza ca fiind procesat, si il citeste


        bool isFileToProcess = false;
        pthread_mutex_lock(&mutexesMapperInputFileNames->at(i));
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
            
            pthread_mutex_lock(mutexMapperResults);
            mapperResults->at(mapper_ID).push_back(MapperElement(word, file_ID));
            pthread_mutex_unlock(mutexMapperResults);
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

    ReducerThreadArgument* threadArgument = (ReducerThreadArgument*) arg;


    int numMappers = threadArgument->numMappers;

    int *numCompletedMappers = threadArgument->numCompletedMappers;
    pthread_mutex_t* mutexNumCompletedMappers = threadArgument->mutexNumCompletedMappers;

    pthread_cond_t* condCompletedMappers = threadArgument->condCompletedMappers;

    vector<pthread_mutex_t>* mutexesProcessedMapperResults = threadArgument->mutexesProcessedMapperResults;
    vector<bool>* isProcessedMapperResults = threadArgument->isProcessedMapperResults;
    vector<MapperResult>* mapperResults = threadArgument->mapperResults;


    pthread_mutex_t* mutexWordList = threadArgument->mutexWordList;
    WordList* wordList = threadArgument->wordList;

    pthread_barrier_t* barrierComputeWordList = threadArgument->barrierComputeWordList;

    pthread_mutex_lock(mutexNumCompletedMappers);

    while ((*numCompletedMappers) != numMappers) {
        pthread_cond_wait(condCompletedMappers, mutexNumCompletedMappers);
    }

    pthread_mutex_unlock(mutexNumCompletedMappers);


    // De abia dupa ce thread-urile Mapper s-au terminat, Reducerii isi pot incepe munca

    for (int i = 0; i < numMappers; i++) {
        bool isMapperResultToProcess = false;
        pthread_mutex_lock(&mutexesProcessedMapperResults->at(i));
        if (isProcessedMapperResults->at(i) == false) {
            isMapperResultToProcess = true;
            isProcessedMapperResults->at(i) = true;
        }
        pthread_mutex_unlock(&mutexesProcessedMapperResults->at(i));

        if (isMapperResultToProcess == false) {
            continue;
        }



        pthread_mutex_lock(mutexWordList);
        for (MapperElement &elem : mapperResults->at(i)) {
            insert_in_word_list(wordList, elem);
        }
        pthread_mutex_unlock(mutexWordList);
    }

    pthread_barrier_wait(barrierComputeWordList);


    // TODO: write wordList to output files

    // Un singur thread face impartirea listei pentru fiecare litera din alfabet


    delete threadArgument;
    pthread_exit(NULL);
}




void print_map_results(vector<MapperResult> &mapperResults)
{
    for (unsigned int i = 0; i < mapperResults.size(); i++) {
        cout << "Mapper " << i << ":\n";
        for (MapperElement &pair : mapperResults[i]) {
            cout << "{ " << pair.word << ", " << pair.fileID << " }\n";
        }
        cout << "\n";
    }
}



void print_wordList(WordList &wordList)
{
    cout << "Word List:\n";

    for (WordListElement &elem : wordList) {
        cout << elem.word << "-> ";
        for (int fileID : elem.fileIDs) {
            cout << fileID << ", ";
        }
        cout << "\n";
    }
    cout << "\n";
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
    pthread_barrier_t barrierComputeWordList;

    pthread_mutex_t mutexMapperResults;
    vector<pthread_mutex_t> mutexesMapperResults;
    pthread_mutex_init(&mutexMapperResults, NULL);



    pthread_cond_t condCompletedMappers;
    pthread_cond_init(&condCompletedMappers, NULL);

    


    threads.resize(numThreads);

    mutexesMapperInputFileNames.resize(numMapperInputFiles);

    for (int i = 0; i < numMapperInputFiles; i++) {
        pthread_mutex_init(&mutexesMapperInputFileNames[i], NULL);
    }
    pthread_barrier_init(&barrierComputeWordList, NULL, numReducers);

    vector<bool> isProcessedMapperFile;
    isProcessedMapperFile.resize(numMapperInputFiles);
    for (int i = 0; i < numMapperInputFiles; i++) {
        isProcessedMapperFile[i] = false;
    }


    vector<MapperResult> mapperResults;
    mapperResults.resize(numMappers);

    vector<bool> isProcessedMapperResults;
    isProcessedMapperResults.resize(numMappers);


    vector<pthread_mutex_t> mutexesProcessedMapperResults;



    mutexesMapperResults.resize(numMappers);
    mutexesProcessedMapperResults.resize(numMappers);
    
    for (int i = 0; i < numMappers; i++) {
        isProcessedMapperResults[i] = false;
        pthread_mutex_init(&mutexesMapperResults[i], NULL);
        pthread_mutex_init(&mutexesProcessedMapperResults[i], NULL);
    }

    WordList wordList;
    pthread_mutex_t mutexWordList;
    pthread_mutex_init(&mutexWordList, NULL);


    int ret_code = 0;

    int numCompletedMappers = 0;
    pthread_mutex_t mutexNumCompletedMappers;
    pthread_mutex_init(&mutexNumCompletedMappers, NULL);

    pthread_mutex_init(&printmtx, NULL);

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
            threadArgument->numInputFiles = numMapperInputFiles; 
            threadArgument->mutexesMapperInputFileNames = &mutexesMapperInputFileNames;
            threadArgument->isProcessedFile = &isProcessedMapperFile;
            threadArgument->inputFileNames = &mapperInputFileNames;
            threadArgument->condCompletedMappers = &condCompletedMappers;
            threadArgument->mutexNumCompletedMappers = &mutexNumCompletedMappers;
            threadArgument->mapperResults = &mapperResults;
            threadArgument->mutexMapperResults = &mutexMapperResults;

            ret_code = pthread_create(&threads[i], NULL, thread_mapper_function, (void*) threadArgument);
        } else {
            ReducerThreadArgument* threadArgument = new ReducerThreadArgument();
            threadArgument->numMappers = numMappers;
            threadArgument->numReducers = numReducers;
            threadArgument->thread_ID = i;
            threadArgument->threadPurpose = REDUCER_THREAD;
            threadArgument->numCompletedMappers = &numCompletedMappers;
            threadArgument->mutexNumCompletedMappers = &mutexNumCompletedMappers;
            threadArgument->condCompletedMappers = &condCompletedMappers;
            threadArgument->mapperResults = &mapperResults;
            threadArgument->mutexesMapperResults = &mutexesMapperResults;
            threadArgument->wordList = &wordList;
            threadArgument->mutexWordList = &mutexWordList;
            threadArgument->isProcessedMapperResults = &isProcessedMapperResults;
            threadArgument->mutexesProcessedMapperResults = &mutexesProcessedMapperResults;
            threadArgument->barrierComputeWordList = &barrierComputeWordList;

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

    for (int i = 0; i < numMappers; i++) {
        for (MapperElement &elem : mapperResults[i]) {
            insert_in_word_list(&wordList, elem);
        }
    }

    // for debugging: print_map_results(mapperResults);
    // for deubgging:  print_wordList(wordList);
    print_map_results(mapperResults);
    print_wordList(wordList);

    pthread_cond_destroy(&condCompletedMappers);

    for (int i = 0; i < numMapperInputFiles; i++) {
        pthread_mutex_destroy(&mutexesMapperInputFileNames[i]);
    }

    for (int i = 0; i < numMappers; i++) {
        pthread_mutex_destroy(&mutexesMapperResults[i]);
        pthread_mutex_destroy(&mutexesProcessedMapperResults[i]);
    }

    pthread_mutex_destroy(&mutexMapperResults);

    pthread_mutex_destroy(&mutexWordList);
    pthread_mutex_destroy(&mutexNumCompletedMappers);

    pthread_barrier_destroy(&barrierComputeWordList);
    
    pthread_mutex_destroy(&printmtx);

    return 0;
}

