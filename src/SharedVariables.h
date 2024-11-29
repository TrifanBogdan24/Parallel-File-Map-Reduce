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

#include "WordList.h"
#include "MapperThread.h"
#include "ReducerThread.h"


using namespace std;



class SharedVariables {

 public:
   int numMappers;
   int numReducers;
   vector<string> inputFileNames;
 private:
   int numInputFiles;

 private:
   vector<pthread_mutex_t> mutexesInputFileNames;
   vector<bool> isProcessedInputFile;

   pthread_mutex_t mutexMapperResults;
   vector<pthread_mutex_t> mutexesMapperResults;   


   vector<MapperResult> mapperResults;
   vector<pthread_mutex_t> mutexesProcessedMapperResults;
   vector<bool> isProcessedMapperResults;

   pthread_mutex_t mutexWordList;
   pthread_barrier_t barrierComputeWordList;
   WordList wordList;

  pthread_cond_t condCompletedMappers;
  pthread_mutex_t mutexNumCompletedMappers;
  int numCompletedMappers;

 public:
    SharedVariables(const int value_numMapper, const int value_numReducers, const vector<string> &values_inputFileNames);
    ~SharedVariables();

 public:
    MapperThread createMapperThread();
    ReducerThread createReducerThread();
};