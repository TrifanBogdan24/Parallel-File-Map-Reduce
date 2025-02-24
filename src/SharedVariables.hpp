#pragma once

// C++ libraries
#include <string>
#include <vector>
#include <queue>

// C libraries
#include <pthread.h>


#include "WordList.hpp"
#include "MapperThread.hpp"
#include "ReducerThread.hpp"


using namespace std;



class SharedVariables {
 private:
   int numMappers;
   int numReducers;
   vector<string> inputFileNames;
 private:
   int numInputFiles;

 private:
   queue<int> queueInputFileIndices;
   pthread_mutex_t mutexQueueInputFileIndices;


   queue<int> queueOutputFileIndices;
   pthread_mutex_t mutexQueueOuputFileIndices;
  

   queue<int> queueMapperResultIndices;
   pthread_mutex_t mutexQueueMapperResultIndices;
   vector<MapperResult> mapperResults;

   pthread_barrier_t barrierComputeWordList;

   vector<pthread_mutex_t> mutexesInsertInWordListConcatenation;
   WordList wordList;

   pthread_mutex_t mutexIsCreatedLetterChuncks;
   int isCreatedLetterChuncks;         // bool


   pthread_cond_t condCompletedMappers;
   pthread_mutex_t mutexNumCompletedMappers;
   int numCompletedMappers;




 public:
    SharedVariables(const int value_numMappers, const int value_numReducers, const vector<string> &values_inputFileNames);
    ~SharedVariables();

 public:
    MapperThread* createMapperThread(int ID_mappperThread);
    ReducerThread* createReducerThread(int ID_reducerThread);


 public:
  void printMapResultsToStdout();
  void writeWordListToFile();
  void printWordListToStdout();
};