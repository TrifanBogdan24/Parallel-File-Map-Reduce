// C++ libraries
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>


// C libraries
#include <pthread.h>
#include <stdlib.h>
#include <cstring>
#include <cctype>
#include <unistd.h>

#include "Boolean.h"
#include "MapperThread.h"
#include "MapperResult.h"




using namespace std;



// Functia de operatie Mapper care se va executa in paralel
void* MapperThread::routine(void *arg)
{
    MapperThread* mapperThread = (MapperThread*) arg;


    while (true) {
        bool isEmptyInputFileQueue = false;
        int fileIndex = 0;

        pthread_mutex_lock(mapperThread->mutexQueueInputFileIndices);
        if (mapperThread->queueInputFileIndices->size() > 0) {
            fileIndex = mapperThread->queueInputFileIndices->front();
            mapperThread->queueInputFileIndices->pop();
        } else {
            isEmptyInputFileQueue = true;
        }
        pthread_mutex_unlock(mapperThread->mutexQueueInputFileIndices);


        if (isEmptyInputFileQueue == true) {
            break;
        }



        // Citeste continutul fisierului
        string inputFileName = mapperThread->inputFileNames->at(fileIndex);
        set<string> uniqueWords = mapperThread->getUniqueWordsInFile(inputFileName);


        for (set<string>::iterator itr = uniqueWords.begin(); itr != uniqueWords.end(); itr++) {
            string word = *itr;
            int file_ID = fileIndex + 1;
            
            mapperThread
                ->mapperResults[mapperThread->mapper_ID]
                ->mapperResultEntries.push_back(MapperResultEntry(word, file_ID));
        }
    }

    pthread_mutex_lock(mapperThread->mutexNumCompletedMappers);

    *(mapperThread->numCompletedMappers) += 1;
    if (*(mapperThread->numCompletedMappers) == mapperThread->numMappers) {
        pthread_cond_broadcast(mapperThread->condCompletedMappers);
    }

    pthread_mutex_unlock(mapperThread->mutexNumCompletedMappers);


    pthread_exit(NULL);
}




set<string> MapperThread::getUniqueWordsInFile(string &inputFileName)
{
    ifstream fin(inputFileName);

    if (!fin.is_open()) {
        return set<string>();
    }

    set<string> uniqueWords;
    string line;

    while (getline(fin, line)) {
        string word;

        for (char &ch : line) {
            if (ch == ' ' || ch == '\t') {
                if (word.empty()) {
                    continue;
                }

                uniqueWords.insert(word);
                word.clear();
                continue;
            } else if (isalpha(ch)) {
                word.push_back(tolower(ch));
            }
        }

        if (!word.empty()) {
            uniqueWords.insert(word);
        }
    }

    fin.close();
    return uniqueWords;
}
