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


#define NUM_ALPHABET_LETTERS (int) 26

class SharedVariables {
 private:
   int numMappers;
   int numReducers;
   vector<string> inputFileNames;
 private:
   int numInputFiles;

 private:
   vector<pthread_mutex_t> mutexesInputFileNames;
   vector<int> isProcessedInputFile;                       // bool

   pthread_mutex_t mutexMapperResults;
   vector<pthread_mutex_t> mutexesMapperResults;   


   vector<MapperResult> mapperResults;
   vector<pthread_mutex_t> mutexesProcessedMapperResults;
   vector<int> isProcessedMapperResults;                  // bool

   pthread_mutex_t mutexWordList;
   pthread_barrier_t barrierComputeWordList;
   WordList wordList;

  pthread_cond_t condCompletedMappers;
  pthread_mutex_t mutexNumCompletedMappers;
  int numCompletedMappers;

  vector<pthread_mutex_t> mutexesIsWrittenOutputFile;
  vector<int> isWrittenOutputFile;                          // bool


 public:
    SharedVariables(const int value_numMappers, const int value_numReducers, const vector<string> &values_inputFileNames);
    ~SharedVariables();

 public:
    MapperThread* createMapperThread(int ID_mappperThread);
    ReducerThread* createReducerThread(int ID_reducerThread);


 public:
  void writeWordList();
  void printMapResults();
  void printWordList();
 

};