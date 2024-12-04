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

#include "SharedVariables.h"
#include "Boolean.h"
#include "WordList.h"
#include "MapperThread.h"
#include "ReducerThread.h"


using namespace std;



SharedVariables::SharedVariables(const int value_numMappers, const int value_numReducers, const vector<string> &values_inputFileNames)
{
    this->numMappers = value_numMappers;
    this->numReducers = value_numReducers;
    this->inputFileNames = values_inputFileNames;

    this->numInputFiles = inputFileNames.size();

    this->numCompletedMappers = 0;


    pthread_cond_init(&condCompletedMappers, NULL);
    pthread_barrier_init(&barrierComputeWordList, NULL, numReducers);

    mutexesInputFileNames.resize(numInputFiles);
    isProcessedInputFile.resize(numInputFiles);

    for (int i = 0; i < numInputFiles; i++) {
        pthread_mutex_init(&mutexesInputFileNames[i], NULL);
        isProcessedInputFile[i] = FALSE;
    }

    mutexesMapperResults.resize(numMappers);
    mapperResults.resize(numMappers);

    mutexesProcessedMapperResults.resize(numMappers);
    isProcessedMapperResults.resize(numMappers);

    isWrittenOutputFile.resize(NUM_ALPHABET_LETTERS);
    mutexesIsWrittenOutputFile.resize(NUM_ALPHABET_LETTERS);

    for (int i = 0; i < numMappers; i++) {
        isProcessedMapperResults[i] = FALSE;
        pthread_mutex_init(&mutexesMapperResults[i], NULL);
        pthread_mutex_init(&mutexesProcessedMapperResults[i], NULL);
    }


    for (int i = 0; i < NUM_ALPHABET_LETTERS; i++) {
        isWrittenOutputFile[i] = 0;
        pthread_mutex_init(&mutexesIsWrittenOutputFile[i], NULL);
    }


    pthread_mutex_init(&mutexWordList, NULL);
    pthread_mutex_init(&mutexNumCompletedMappers, NULL);
    pthread_mutex_init(&mutexMapperResults, NULL);
}


SharedVariables::~SharedVariables()
{
    pthread_cond_destroy(&condCompletedMappers);
    pthread_barrier_destroy(&barrierComputeWordList);    

    for (int i = 0; i < numInputFiles; i++) {
        pthread_mutex_destroy(&mutexesInputFileNames[i]);
    }

    for (int i = 0; i < numMappers; i++) {
        pthread_mutex_destroy(&mutexesMapperResults[i]);
        pthread_mutex_destroy(&mutexesProcessedMapperResults[i]);
    }

    for (int i = 0; i < NUM_ALPHABET_LETTERS; i++) {
        pthread_mutex_destroy(&mutexesIsWrittenOutputFile[i]);
    }

    pthread_mutex_destroy(&mutexMapperResults);

    pthread_mutex_destroy(&mutexWordList);
    pthread_mutex_destroy(&mutexNumCompletedMappers);
}


MapperThread* SharedVariables::createMapperThread(int ID_mappperThread)
{
    MapperThread* mapperThread = new MapperThread();

    mapperThread->mapper_ID = ID_mappperThread;

    mapperThread->numMappers = this->numMappers;
    mapperThread->numReducers = this->numReducers;
    mapperThread->numInputFiles = this->numInputFiles;
    mapperThread->numCompletedMappers = &this->numCompletedMappers;
    mapperThread->inputFileNames = &this->inputFileNames;
    

    mapperThread->mutexesInputFiles.resize(numInputFiles);
    mapperThread->isProcessedInputFile.resize(numInputFiles);


    for (int i = 0; i < numInputFiles; i++) {
        mapperThread->isProcessedInputFile[i] = &(this->isProcessedInputFile[i]);
        mapperThread->mutexesInputFiles[i] = &(this->mutexesInputFileNames[i]);
    }



    mapperThread->condCompletedMappers = &this->condCompletedMappers;
    mapperThread->mutexNumCompletedMappers = &this->mutexNumCompletedMappers;

    mapperThread->mapperResults.resize(numMappers);

    for (int i = 0; i < numMappers; i++) {
        mapperThread->mapperResults[i] = &(this->mapperResults[i]);
    }

    return mapperThread;
}

ReducerThread* SharedVariables::createReducerThread(int ID_reducerThread)
{
    ReducerThread* reducerThread = new ReducerThread();

    reducerThread->reducer_ID = ID_reducerThread;
    reducerThread->numMappers = this->numMappers;
    reducerThread->numReducers = this->numReducers;
    reducerThread->numCompletedMappers = &this->numCompletedMappers;
    reducerThread->mutexNumCompletedMappers = &this->mutexNumCompletedMappers;
    reducerThread->condCompletedMappers = &this->condCompletedMappers;
    reducerThread->wordList = &this->wordList;
    reducerThread->mutexWordList = &this->mutexWordList;

    reducerThread->isProcessedMapperResults.resize(numMappers);    
    reducerThread->mutexesProcessedMapperResults.resize(numMappers);
    reducerThread->mapperResults.resize(numMappers);


    for (int i = 0; i < numMappers; i++) {
        reducerThread->isProcessedMapperResults[i] = &(this->isProcessedMapperResults[i]);
        reducerThread->mutexesProcessedMapperResults[i] = &(this->mutexesProcessedMapperResults[i]);
        reducerThread->mapperResults[i] = &(this->mapperResults[i]);
    }


    reducerThread->barrierComputeWordList = &this->barrierComputeWordList;


    reducerThread->isWrittenOutputFile.resize(NUM_ALPHABET_LETTERS);
    reducerThread->mutexesIsWrittenOutputFile.resize(NUM_ALPHABET_LETTERS);

    for (int i = 0; i < NUM_ALPHABET_LETTERS; i++) {
        reducerThread->isWrittenOutputFile[i] = &(this->isWrittenOutputFile[i]);
        reducerThread->mutexesIsWrittenOutputFile[i] = &(this->mutexesIsWrittenOutputFile[i]);
    }

    return reducerThread;
}




void SharedVariables::printMapResults()
{
    
    for (unsigned int i = 0; i < this->mapperResults.size(); i++) {
        cout << "Mapper " << i << ":\n";
        for (MapperResultEntry &pair : mapperResults[i].mapperResultEntries) {
            cout << "{ " << pair.word << ", " << pair.fileID << " }\n";
        }
        cout << "\n";
    }
}

void SharedVariables::printWordList()
{
    this->wordList.printWordList();
}
