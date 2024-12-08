#pragma once

// C++ libraries
#include <string>
#include <vector>
#include <queue>

// C libraries
#include <pthread.h>

#include "MapperResult.h"
#include "WordList.h"

#define NUM_ALPHABET_LETTERS (int) 26


using namespace std;

class ReducerThread {
 public:
   int thread_ID;
   int numMappers;
   int numReducers;
 

 public:
   int reducer_ID;
   int numInputFiles;

   int* numCompletedMappers;
   pthread_mutex_t* mutexNumCompletedMappers;
   pthread_cond_t* condCompletedMappers;

   queue<int>* queueMapperResultIndices;
   pthread_mutex_t* mutexQueueMapperResultIndices;
   vector<MapperResult>* mapperResults;

   vector<pthread_mutex_t*> mutexesInsertInWordListConcatenation;
   WordList* wordList;

   pthread_mutex_t* mutexIsCreatedLetterChuncks;
   int* isCreatedLetterChuncks;            // bool


   pthread_barrier_t* barrierComputeWordList;


   queue<int>* queueOutputFileIndices;
   pthread_mutex_t* mutexQueueOutputFileIndices;





 public:
   // SharedVariables se ocupa de initializarea valorilor thread-ului
   ReducerThread() {}
   ~ReducerThread() {}

 public:
    static void* routine(void *arg);

};