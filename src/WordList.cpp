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

#include "WordList.h"
#include "MapperResult.h"


using namespace std;





void WordList::insertInWordList(MapperResultEntry &mapperEntry)
{
    string word = mapperEntry.word;
    int fileID = mapperEntry.fileID;

    int length = (int) wordListEntries.size();

    if (length == 0) {
        this->wordListEntries.push_back(WordListEntry(word, {fileID}));
        return;
    }

    if (word < this->wordListEntries[0].word) {
        this->wordListEntries.insert(wordListEntries.begin(), WordListEntry(word, {fileID}));
        return;
    }

    if (word > this->wordListEntries[length - 1].word) {
        this->wordListEntries.push_back(WordListEntry(word, {fileID}));
        return;
    }


    int left = 0;
    int right = length - 1;
    int middle = 0;

    while (left <= right) {
        middle = left + (right - left) / 2;

        
        if (this->wordListEntries[middle].word < word) {
            left = middle + 1;
        } else if (this->wordListEntries[middle].word > word) {
            right = middle - 1;
        } else {
            // then: this->wordListEntries[middle].word == word
            // Updating word's set
            this->wordListEntries[middle].fileIDs.insert(fileID);
            return;
        }
    }

    this->wordListEntries.insert(wordListEntries.begin() + left, WordListEntry(word, {fileID}));
}



optional<WordListChunck> WordList::findChunkOfCharacterInWordList(char ch)
{
    int firstIndex = -1;
    int lastIndex = -1;
    int left = 0;
    int right = 0;
    int middle = 0;
    

    // Gasirea indexului primului cuvant care incepe cu caracterul "ch"
    left = 0;
    right = this->wordListEntries.size() - 1;

    while (left <= right) {
        middle = left + (right - left) / 2;
        
        if (this->wordListEntries[middle].word[0] < ch) {
            left = middle + 1;
        } else if (this->wordListEntries[middle].word[0] > ch) {
            right = middle + 1;
        } else {
            // then: this->wordListEntries[middle].word[0] == ch)
            firstIndex = middle;
            right = middle -1;
        }
    }


    if (firstIndex == -1) {
        return nullopt;
    }


    // Gasirea indexului ultimului cuvant care incepe cu caracterul "ch"
    left = 0;
    right = this->wordListEntries.size() - 1;

    while (left <= right) {
        middle = left + (right - left) / 2;

        if (wordListEntries[middle].word[0] < ch) {
            left = middle + 1;
        } else if (wordListEntries[middle].word[0] > ch) {
            right = middle - 1;
        } else {
            // then: wordListEntries[middle].word[0] == ch
            lastIndex = middle;
            left = middle + 1;
        }
    }


    if (lastIndex == -1) {
        return nullopt;
    }

    return WordListChunck(ch, firstIndex, lastIndex);
}

