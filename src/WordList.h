#pragma once

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

#include "MapperResult.h"

using namespace std;

class WordListEntry {
 public:
    string word;
    set<int> fileIDs;

 public:
    WordListEntry(const string &wordValue) :
        word(wordValue)
    {
    }

    WordListEntry(const string &wordValue, const set<int> &fileIDValues):
        word(wordValue), fileIDs(fileIDValues)
    {
    }


    ~WordListEntry()
    {
    }
};


class WordListChunck {
 public:
    char ch;
    int firstIndex;
    int lastIndex;

 public:
    WordListChunck(char ch_value, int firstIndex_value, int lastIndex_value):
        ch(ch_value), firstIndex(firstIndex_value), lastIndex(lastIndex_value)
    {
    }

    ~WordListChunck() {}
};


class WordList {
 private:
    vector<WordListEntry> wordListEntries;
 
 public:
    WordList() {}
    ~WordList() {}

 public:
    void insertInWordList(MapperResultEntry &mapperEntry);
 private:
   WordListChunck findChunkOfCharacterInWordList(char letter);
   static bool compareEntries(const WordListEntry &entry1, const WordListEntry &entry2);
   void writeInputFileIDs(ostream &fout, set<int> &fileIDs);
   void writeWordListEntry(ostream &fout, WordListEntry &wordListEntry);
 public:
   void writeLetterChunck(char letter);

 public:
   void writeWordList();  // to word-list.txt
   void printWordList();

};
