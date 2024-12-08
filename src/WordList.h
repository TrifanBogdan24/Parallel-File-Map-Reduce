#pragma once

// C++ libraries
#include <map>
#include <set>
#include <string>
#include <vector>



#include "MapperResult.h"

#define NUM_ALPHABET_LETTERS (int) 26


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


class WordListLetterChunck {
 public:
   vector<WordListEntry> letterChunckEntries;

 public:
    WordListLetterChunck() {}
    ~WordListLetterChunck() {}
};


class WordList {
 public:
    vector<map<string, set<int>>> mapperResultConcatenation;
    vector<WordListLetterChunck> wordListLetterChuncks;
 
 public:
    WordList()
    {
      this->mapperResultConcatenation.resize(NUM_ALPHABET_LETTERS);
      this->wordListLetterChuncks.resize(NUM_ALPHABET_LETTERS);
    }
    ~WordList() {}

 public:
    void insertInMapperResultConcatenation(MapperResultEntry &mapperResultEntry);
    void createLetterChunck(int idxFirstWordLetter);
    void sortLetterChunck(int idxFirstWordLetter);

 private:
   static bool compareEntries(const WordListEntry &entry1, const WordListEntry &entry2);
   void writeInputFileIDs(ostream &fout, set<int> &fileIDs);
   void writeWordListEntry(ostream &fout, WordListEntry &wordListEntry);
 public:
   void writeLetterChunck(int idxFirstWordLetter);


 public:
   void writeWordList(ostream& fout);
   void writeWordListToFile();  // to word-list.txt
   void printWordListToStdout();
};
