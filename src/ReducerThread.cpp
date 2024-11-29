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


#include "ReducerThread.h"
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

    for (int i = 0; i < reducerThread->numMappers; i++) {
        bool isMapperResultToProcess = false;
        
        pthread_mutex_lock(&reducerThread->mutexesProcessedMapperResults->at(i));
        if (reducerThread->isProcessedMapperResults->at(i) == false) {
            isMapperResultToProcess = true;
            reducerThread->isProcessedMapperResults->at(i) = true;
        }
        pthread_mutex_unlock(&reducerThread->mutexesProcessedMapperResults->at(i));

        if (isMapperResultToProcess == false) {
            continue;
        }



        // pthread_mutex_lock(reducerThread->mutexWordList);
        // for (MapperResultEntry &elem : reducerThread->mapperResults->at(i)) {
        //     reducerThread->wordList->insertInWordList(elem);
        // }
        // pthread_mutex_unlock(reducerThread->mutexWordList);
    }

    pthread_barrier_wait(reducerThread->barrierComputeWordList);



    pthread_exit(NULL);
}

