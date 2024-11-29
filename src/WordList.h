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
    optional<WordListChunck> findChunkOfCharacterInWordList(char ch);
};