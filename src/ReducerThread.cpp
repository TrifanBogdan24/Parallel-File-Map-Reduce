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

    for (int i = 0; i < reducerThread->numMappers; i++) {
        int isMapperResultToProcess = FALSE;
        
        pthread_mutex_lock(reducerThread->mutexesProcessedMapperResults[i]);
        if (*(reducerThread->isProcessedMapperResults[i]) == FALSE) {
            isMapperResultToProcess = TRUE;
            *(reducerThread->isProcessedMapperResults[i]) = TRUE;
        }
        pthread_mutex_unlock(reducerThread->mutexesProcessedMapperResults[i]);

        if (isMapperResultToProcess == FALSE) {
            continue;
        }




        for (MapperResultEntry &elem : reducerThread->mapperResults[i]->mapperResultEntries) {
            int idxFirstWordLetter = elem.word[0] - 'a';
            pthread_mutex_lock(reducerThread->mutexesWordListLetterChuncks[idxFirstWordLetter]);
            reducerThread->wordList->insertInMapperResultConcatenation(elem);
            pthread_mutex_unlock(reducerThread->mutexesWordListLetterChuncks[idxFirstWordLetter]);
        }

    }


    pthread_barrier_wait(reducerThread->barrierComputeWordList);



    // WordList-ul a fost creat; acum trebuie sa scriem din el in fisiere

    for (int i = 0; i < NUM_ALPHABET_LETTERS; i++) {
        int isOutputFileToWrite = FALSE;

        pthread_mutex_lock(reducerThread->mutexesIsWrittenOutputFile[i]);
        if (*(reducerThread->isWrittenOutputFile[i]) == FALSE) {
            isOutputFileToWrite = TRUE;
        }
        pthread_mutex_unlock(reducerThread->mutexesIsWrittenOutputFile[i]);


        if (isOutputFileToWrite == FALSE) {
            continue;
        }



        // reducerThread->wordList->createLetterChunck(i);
        // reducerThread->wordList->sortLetterChunck(i);
        // reducerThread->wordList->writeLetterChunck(i);
    }


    pthread_exit(NULL);
}




