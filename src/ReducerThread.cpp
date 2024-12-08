// C++ libraries
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>

// C libraries
#include <pthread.h>
#include <stdlib.h>
#include <cstring>
#include <cctype>
#include <unistd.h>


#include "ReducerThread.h"
#include "Boolean.h"
#include "MapperResult.h"


using namespace std;

// Functia de operatie Reducer care se va executa in paralel
void* ReducerThread::routine(void *arg)
{
    ReducerThread* reducerThread = (ReducerThread*) arg;

    pthread_mutex_lock(reducerThread->mutexNumCompletedMappers);
    while (*(reducerThread->numCompletedMappers) != reducerThread->numMappers) {
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


