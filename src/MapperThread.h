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

#include "MapperResult.h"


using namespace std;



class MapperThread {
 public:
    int thread_ID;
    int numMappers;
    int numReducers;
    
    int *numCompletedMappers;
    pthread_mutex_t* mutexNumCompletedMappers;
    pthread_cond_t* condCompletedMappers;



 public:
    int mapper_ID;
    int numInputFiles;
    vector<pthread_mutex_t*> mutexesInputFiles;
    vector<int*> isProcessedInputFile;             // bool
    vector<string>* inputFileNames;
    vector<MapperResult*> mapperResults;
    

 
 public:
   // SharedVariables se ocupa de initializarea valorilor thread-ului
    MapperThread() {}
    ~MapperThread() {}

 public:
   static void* routine(void *arg);
 
 private:
   set<string> getUniqueWordsInFile(string &inputFileName);


};