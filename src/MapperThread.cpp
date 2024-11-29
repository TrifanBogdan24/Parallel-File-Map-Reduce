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


#include "MapperThread.h"
#include "MapperResult.h"

using namespace std;



// Functia de operatie Mapper care se va executa in paralel
void* MapperThread::routine(void *arg)
{
    MapperThread* mapperThread = (MapperThread*) arg;


    for (int i = 0; i < mapperThread->numInputFiles; i++) {
        // Thread-ul va lua un fisier care nu a fost citit deja, il marcheaza ca fiind procesat, si il citeste


        bool isFileToProcess = false;

        pthread_mutex_lock(&mapperThread->mutexesInputFiles->at(i));
        if (mapperThread->isProcessedInputFile->at(i) == false) {
            // Daca gasesc ca un fisier nu a fost procesat, il marchez si il citesc
            isFileToProcess = true;
            mapperThread->isProcessedInputFile->at(i) = true;
        }
        pthread_mutex_unlock(&mapperThread->mutexesInputFiles->at(i));



        if (!isFileToProcess) {
            continue;
        }

        // // Citeste continutul fisierului
        string inputFileName = mapperThread->inputFileNames->at(i);
        set<string> uniqueWords = getUniqueWordsInFile(inputFileName);


        
        for (set<string>::iterator itr = uniqueWords.begin(); itr != uniqueWords.end(); itr++) {
            string word = *itr;
            int file_ID = i;
            
            mapperThread
                ->mapperResults->at(mapperThread->mapper_ID)
                .mapperResultEntries.push_back(MapperResultEntry(word, file_ID));
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
        cerr << "[ERROR] Cannot open input file <" << inputFileName << ">\n";
        // Return an empty set
        return set<string>();
    }

    set<string> uniqueWords;


    string line;

    while (getline(fin, line)) {
        for (char& chr : line) {
            chr = isalpha(chr) ? tolower(chr) : ' ';
        }

        char* str = strtok(&line[0], " \t\n");

        while (str != NULL) {
            string word(str);

            if (!word.empty()) {
                uniqueWords.insert(word);
            }

            str = strtok(NULL, " \t\n");
        }

    }


    fin.close();
    return uniqueWords;
}

