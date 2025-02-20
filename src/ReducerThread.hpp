#pragma once

// C++ libraries
#include <string>
#include <vector>
#include <queue>

// C libraries
#include <pthread.h>

#include "MapperResult.hpp"
#include "WordList.hpp"

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
   static void* routine(void *arg)
   {
    ReducerThread* reducerThread = (ReducerThread*) arg;
  
    pthread_mutex_lock(reducerThread->mutexNumCompletedMappers);
    if (*(reducerThread->numCompletedMappers) != reducerThread->numMappers) {
      pthread_cond_wait(reducerThread->condCompletedMappers, reducerThread->mutexNumCompletedMappers);
    }
    pthread_mutex_unlock(reducerThread->mutexNumCompletedMappers);
  
  
    // De abia dupa ce thread-urile Mapper s-au terminat, Reducerii isi pot incepe munca
  
  
    while (true) {
      bool isEmptyMapperResultQueue = false;
      int mapperResultIndex = 0;
  
      pthread_mutex_lock(reducerThread->mutexQueueMapperResultIndices);
      if (reducerThread->queueMapperResultIndices->size() > 0) {
        mapperResultIndex = reducerThread->queueMapperResultIndices->front();
        reducerThread->queueMapperResultIndices->pop();
      } else {
        isEmptyMapperResultQueue = true;
      }
      pthread_mutex_unlock(reducerThread->mutexQueueMapperResultIndices);
  
  
      if (isEmptyMapperResultQueue == true) {
          break;
      }
  
  
      for (MapperResultEntry &elem : reducerThread->mapperResults->at(mapperResultIndex).mapperResultEntries) {
        int idxFirstWordLetter = elem.word[0] - 'a';
        pthread_mutex_lock(reducerThread->mutexesInsertInWordListConcatenation[idxFirstWordLetter]);
        reducerThread->wordList->insertInMapperResultConcatenation(elem);
        pthread_mutex_unlock(reducerThread->mutexesInsertInWordListConcatenation[idxFirstWordLetter]);
      }
  
    }
  
  
  
    pthread_barrier_wait(reducerThread->barrierComputeWordList);
  
  
  
  
    // WordList-ul a fost creat; acum trebuie sa scriem din el in fisiere
  
    while (true) {
      bool isEmptyOutputFileQueue = false;
      int letterIndex = 0;
  
      pthread_mutex_lock(reducerThread->mutexQueueOutputFileIndices);
      if (reducerThread->queueOutputFileIndices->size() > 0) {
        letterIndex = reducerThread->queueOutputFileIndices->front();
        reducerThread->queueOutputFileIndices->pop();
      } else {
        isEmptyOutputFileQueue = true;
      }
      pthread_mutex_unlock(reducerThread->mutexQueueOutputFileIndices);
  
      if (isEmptyOutputFileQueue == true) {
        break;
      }
  
      reducerThread->wordList->createLetterChunck(letterIndex);
      reducerThread->wordList->sortLetterChunck(letterIndex);
      reducerThread->wordList->writeLetterChunck(letterIndex);
    }
  
  
    pthread_exit(NULL);
  }

 public:
   // SharedVariables se ocupa de initializarea valorilor thread-ului
   ReducerThread() {}
   ~ReducerThread() {}

};





