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
    pthread_mutex_lock(mutexNumCompletedMappers);

    while ((*numCompletedMappers) != numMappers) {
        pthread_cond_wait(condCompletedMappers, mutexNumCompletedMappers);
    }

    pthread_mutex_unlock(mutexNumCompletedMappers);


    // De abia dupa ce thread-urile Mapper s-au terminat, Reducerii isi pot incepe munca

    for (int i = 0; i < numMappers; i++) {
        bool isMapperResultToProcess = false;
        pthread_mutex_lock(&mutexesProcessedMapperResults->at(i));
        if (isProcessedMapperResults->at(i) == false) {
            isMapperResultToProcess = true;
            isProcessedMapperResults->at(i) = true;
        }
        pthread_mutex_unlock(&mutexesProcessedMapperResults->at(i));

        if (isMapperResultToProcess == false) {
            continue;
        }



        // pthread_mutex_lock(mutexWordList);
        // for (MapperResultEntry &elem : mapperResults->at(i)) {
        //     wordList->insertInWordList(elem);
        // }
        // pthread_mutex_unlock(mutexWordList);
    }

    pthread_barrier_wait(barrierComputeWordList);



    pthread_exit(NULL);
}

