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
#include "WordList.h"
#include "MapperThread.h"
#include "ReducerThread.h"


using namespace std;



SharedVariables::SharedVariables(const int value_numMapper, const int value_numReducers, const vector<string> &values_inputFileNames)
{
    this->numMappers = value_numMapper;
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
        isProcessedInputFile[i] = false;
    }


    mapperResults.resize(numMappers);
    isProcessedMapperResults.resize(numMappers);
    mutexesProcessedMapperResults.resize(numMappers);

    for (int i = 0; i < numMappers; i++) {
        isProcessedMapperResults[i] = false;
        pthread_mutex_init(&mutexesMapperResults[i], NULL);
        pthread_mutex_init(&mutexesProcessedMapperResults[i], NULL);
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

    pthread_mutex_destroy(&mutexMapperResults);

    pthread_mutex_destroy(&mutexWordList);
    pthread_mutex_destroy(&mutexNumCompletedMappers);
}


MapperThread SharedVariables::createMapperThread()
{
    MapperThread mapperThread = MapperThread();
    mapperThread.numMappers = this->numMappers;
    mapperThread.numReducers = this->numReducers;
    mapperThread.numInputFiles = this->numInputFiles; 
    mapperThread.numCompletedMappers = &this->numCompletedMappers;
    mapperThread.mutexesInputFiles = &this->mutexesInputFileNames;
    mapperThread.isProcessedInputFile = &this->isProcessedInputFile;
    mapperThread.inputFileNames = &this->inputFileNames;
    mapperThread.condCompletedMappers = &this->condCompletedMappers;
    mapperThread.mutexNumCompletedMappers = &this->mutexNumCompletedMappers;
    mapperThread.mapperResults = &this->mapperResults;
    mapperThread.mutexMapperResults = &this->mutexMapperResults;

    return mapperThread;
}

ReducerThread SharedVariables::createReducerThread()
{
    ReducerThread reducerThread = ReducerThread();
    reducerThread.numMappers = this->numMappers;
    reducerThread.numReducers = this->numReducers;
    reducerThread.numCompletedMappers = &this->numCompletedMappers;
    reducerThread.mutexNumCompletedMappers = &this->mutexNumCompletedMappers;
    reducerThread.condCompletedMappers = &this->condCompletedMappers;
    reducerThread.mapperResults = &this->mapperResults;
    reducerThread.mutexesMapperResults = &this->mutexesMapperResults;
    reducerThread.wordList = &this->wordList;
    reducerThread.mutexWordList = &this->mutexWordList;
    reducerThread.isProcessedMapperResults = &this->isProcessedMapperResults;
    reducerThread.mutexesProcessedMapperResults = &this->mutexesProcessedMapperResults;
    reducerThread.barrierComputeWordList = &this->barrierComputeWordList;

    return reducerThread;
}