// C++ libraries
#include <iostream>
#include <string>
#include <vector>

// C libraries
#include <pthread.h>

#include "SharedVariables.hpp"
#include "WordList.hpp"
#include "MapperThread.hpp"
#include "ReducerThread.hpp"


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


    pthread_mutex_init(&mutexQueueInputFileIndices, NULL);

    for (int i = 0; i < numInputFiles; i++) {
        queueInputFileIndices.push(i);
    }



    mapperResults.resize(numMappers);

    for (int i = 0; i < numMappers; i++) {
        queueMapperResultIndices.push(i);
    }


    pthread_mutex_init(&mutexQueueOuputFileIndices, NULL);

    this->wordList.wordListLetterChuncks.resize(NUM_ALPHABET_LETTERS);
    this->mutexesInsertInWordListConcatenation.resize(NUM_ALPHABET_LETTERS);

    for (int i = 0; i < NUM_ALPHABET_LETTERS; i++) {
        this->queueOutputFileIndices.push(i);
        pthread_mutex_init(&mutexesInsertInWordListConcatenation[i], NULL);
    }


    isCreatedLetterChuncks = 1;
    pthread_mutex_init(&mutexIsCreatedLetterChuncks, NULL);


    pthread_mutex_init(&mutexNumCompletedMappers, NULL);
    pthread_mutex_init(&mutexQueueMapperResultIndices, NULL);
}


SharedVariables::~SharedVariables()
{
    pthread_cond_destroy(&condCompletedMappers);
    pthread_barrier_destroy(&barrierComputeWordList);    

    for (int i = 0; i < NUM_ALPHABET_LETTERS; i++) {
        pthread_mutex_destroy(&mutexesInsertInWordListConcatenation[i]);
    }

    pthread_mutex_destroy(&mutexQueueInputFileIndices);
    pthread_mutex_destroy(&mutexQueueOuputFileIndices);


    pthread_mutex_destroy(&mutexIsCreatedLetterChuncks);

    pthread_mutex_destroy(&mutexQueueMapperResultIndices);

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
    

    mapperThread->queueInputFileIndices = &this->queueInputFileIndices;
    mapperThread->mutexQueueInputFileIndices = &this->mutexQueueInputFileIndices;

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

    reducerThread->queueMapperResultIndices = &this->queueMapperResultIndices;
    reducerThread->mutexQueueMapperResultIndices = &this->mutexQueueMapperResultIndices;
    reducerThread->mapperResults = &this->mapperResults;

    reducerThread->barrierComputeWordList = &this->barrierComputeWordList;

    reducerThread->queueOutputFileIndices = &this->queueOutputFileIndices;
    reducerThread->mutexQueueOutputFileIndices = &this->mutexQueueOuputFileIndices;

    reducerThread->mutexesInsertInWordListConcatenation.resize(NUM_ALPHABET_LETTERS);
    for (int i = 0; i < NUM_ALPHABET_LETTERS; i++) {
        reducerThread->mutexesInsertInWordListConcatenation[i] = &this->mutexesInsertInWordListConcatenation[i];
    }

    reducerThread->mutexIsCreatedLetterChuncks = &this->mutexIsCreatedLetterChuncks;
    reducerThread->isCreatedLetterChuncks = &this->isCreatedLetterChuncks;

    return reducerThread;
}




void SharedVariables::printMapResultsToStdout()
{
    
    for (unsigned int i = 0; i < this->mapperResults.size(); i++) {
        cout << "Mapper " << i << ":\n";
        for (MapperResultEntry &pair : mapperResults[i].mapperResultEntries) {
            cout << "{ " << pair.word << ", " << pair.fileID << " }\n";
        }
        cout << "\n";
    }
}

void SharedVariables::printWordListToStdout()
{
    this->wordList.printWordListToStdout();
}


void SharedVariables::writeWordListToFile()
{
    this->wordList.writeWordListToFile();
}

