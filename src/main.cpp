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


#include "CommandLineArgumentsParser.h"
#include "SharedVariables.h"
#include "MapperThread.h"
#include "ReducerThread.h"

using namespace std;





// pthread_mutex_t printmtx;



// void write_file_IDs(ofstream &fout, vector<int> &fileIDs)
// {

//     for (unsigned int i = 0; i < fileIDs.size(); i++) {
//         fout << fileIDs[i];
//         if (i != fileIDs.size() - 1) {
//             fout << " ";
//         }
//     }
// }

// void write_word_file_IDs(ofstream &fout, WordList &wordList, unsigned int &idx)
// {
//     fout << wordList[idx].word << ": [";
    
//     vector<int> fileIDs(wordList[idx].fileIDs.begin(), wordList[idx].fileIDs.end());
//     sort(fileIDs.begin(), fileIDs.end());

//     write_file_IDs(fout, fileIDs);
//     fout << "]\n";
// }


// void write_word_list_chunk(WordList* wordList)
// {
//     // TODO: fol functiile de mai sus pt scriere
// }




// void print_map_results(vector<MapperResult> &mapperResults)
// {
//     for (unsigned int i = 0; i < mapperResults.size(); i++) {
//         cout << "Mapper " << i << ":\n";
//         for (MapperElement &pair : mapperResults[i]) {
//             cout << "{ " << pair.word << ", " << pair.fileID << " }\n";
//         }
//         cout << "\n";
//     }
// }



// void print_wordList(WordList &wordList)
// {
//     cout << "Word List:\n";

//     for (WordListElement &elem : wordList) {
//         cout << elem.word << "-> ";
//         for (int fileID : elem.fileIDs) {
//             cout << fileID << ", ";
//         }
//         cout << "\n";
//     }
//     cout << "\n";
// }



vector<string> readInptutFile(string &inputFileName)
{
    ifstream fin(inputFileName);

    vector<string> mapperInputFileNames;

    if (!fin.is_open()) {
        cerr << "[ERROR] Cannot open input file <" << inputFileName << ">\n";
        exit(EXIT_FAILURE);
    }


    int numFileNames = 0;
    fin >> numFileNames;
    // Reading the new-line ('\n') character
    fin.get();

    string fileName;

    for (int i = 0; i < numFileNames; i++) {
        getline(fin, fileName);
        mapperInputFileNames.push_back(fileName);
    }

    fin.close();
    return mapperInputFileNames;
}


int main(int argc, char* argv[])
{
    // TODO: make it run with no SEG FAULT

    CommandLineArgumentsParser cliArgsParser = CommandLineArgumentsParser(argc, argv);
    
    int numMappers = cliArgsParser.getNumMappers();            // argv[1]
    int numReducers = cliArgsParser.getNumReducers();          // argv[2]
    string inputFileName = cliArgsParser.getInputFileName();   // argv[3]

    vector<string> mapperInputFileNames = readInptutFile(inputFileName);

    SharedVariables sharedVariables = SharedVariables(numMappers, numReducers, mapperInputFileNames);

    int numThreads = numMappers + numReducers;
    vector<pthread_t> threads;
    threads.resize(numThreads);

    WordList wordList = WordList();


    int ret_code = 0;

    for (int i = 0; i < numThreads; i++) {
        // Dowcasting is slow...I'm not using it
        ret_code = 0;

        if (i < numMappers) {
            MapperThread mapperThread = sharedVariables.createMapperThread();
            ret_code = pthread_create(&threads[i], NULL, mapperThread.startRoutine, NULL);
        } else {
            ReducerThread reducerThread = sharedVariables.createReducerThread();
            ret_code = pthread_create(&threads[i], NULL, reducerThread.startRoutine, NULL);
        }

        if (ret_code) {
            cerr << "[ERROR] Eroare la crearea thread-ului: " << i << "\n";
        }
    }

    void *status;
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], &status);
        
        if (ret_code) {
            cerr << "[ERROR] Eroare la terminarea thread-ului: " << i << "\n";
        }
    }

    // for (int i = 0; i < numMappers; i++) {
    //     for (MapperElement &elem : mapperResults[i]) {
    //         insert_in_word_list(&wordList, elem);
    //     }
    // }

    // // for debugging: print_map_results(mapperResults);
    // // for deubgging:  print_wordList(wordList);
    // print_map_results(mapperResults);
    // print_wordList(wordList);

    return 0;
}

