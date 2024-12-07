#pragma once

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

   int *numCompletedMappers;
   pthread_mutex_t* mutexNumCompletedMappers;
   pthread_cond_t* condCompletedMappers;

   vector<pthread_mutex_t*> mutexesMapperResults;
   vector<MapperResult*> mapperResults;

   vector<int*> isProcessedMapperResults;                   // bool
   vector<pthread_mutex_t*> mutexesProcessedMapperResults;

   vector<pthread_mutex_t*> mutexesWordListLetterChuncks;
   WordList* wordList;

   pthread_barrier_t* barrierComputeWordList;

   pthread_mutex_t* mutexIsCompletedMapperResultsConcatenation;
   int* isCompletedMapperResultsConcatenation;                // bool



   vector<pthread_mutex_t*> mutexesIsWrittenOutputFile;
   vector<int*> isWrittenOutputFile;                        // bool



 public:
   // SharedVariables se ocupa de initializarea valorilor thread-ului
   ReducerThread() {}
   ~ReducerThread() {}

 public:
    static void* routine(void *arg);

};