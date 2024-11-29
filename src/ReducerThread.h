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
    
    vector<pthread_mutex_t>* mutexesMapperResults;
    vector<MapperResult>* mapperResults;

    vector<bool>* isProcessedMapperResults;
    vector<pthread_mutex_t>* mutexesProcessedMapperResults;

    pthread_mutex_t* mutexWordList;
    WordList* wordList;

    pthread_barrier_t* barrierComputeWordList;

 public:
    void* routine(void *arg);

  public:
    static void* startRoutine(void* context) {
        return static_cast<ReducerThread*>(context)->routine(nullptr);
    }
};