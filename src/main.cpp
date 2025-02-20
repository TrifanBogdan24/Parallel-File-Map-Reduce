// C++ libraries
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// C libraries
#include <pthread.h>



#include "CLI-ArgumentsParser.hpp"
#include "SharedVariables.hpp"
#include "MapperThread.hpp"
#include "ReducerThread.hpp"

using namespace std;


vector<string> readInptutFile(string &inputFileName)
{
    ifstream fin(inputFileName);

    vector<string> mapperInputFileNames;

    if (!fin.is_open()) {
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





int main(int argc, char* argv[])
{
    if (validateInput(argc, argv) == false) {
        return EXIT_FAILURE;
    }
    
    int numMappers = charsToInt(argv[1]);
    int numReducers = charsToInt(argv[2]);
    string inputFileName = argv[3];

    vector<string> mapperInputFileNames = readInptutFile(inputFileName);

    int numThreads = numMappers + numReducers;
    vector<pthread_t> threads;
    threads.resize(numThreads);

    WordList wordList = WordList();

    SharedVariables sharedVariables = SharedVariables(numMappers, numReducers, mapperInputFileNames);


    int ret_code = 0;

    for (int i = 0; i < numThreads; i++) {
        // Dowcasting is slow...I'm not using it
        ret_code = 0;

        if (i < numMappers) {
            MapperThread* mapperThread = sharedVariables.createMapperThread(i);
            ret_code = pthread_create(&threads[i], NULL, MapperThread::routine, (void*) mapperThread);
        } else {
            ReducerThread* reducerThread = sharedVariables.createReducerThread(i - numMappers);
            ret_code = pthread_create(&threads[i], NULL, ReducerThread::routine, (void*) reducerThread);
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




    // // for debugging: sharedVariables.printMapResultsToStdou();
    // // for deubgging:  sharedVariables.printWordListToStdout();
    // // for deubgging:  sharedVariables.writeWordListToFile();


    return 0;
}

