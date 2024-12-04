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



optional<WordListChunck> WordList::findChunkOfCharacterInWordList(char letter)
{
    int firstIndex = -1;
    int lastIndex = -1;
    int left = 0;
    int right = 0;
    int middle = 0;
    

    // Gasirea indexului primului cuvant care incepe cu caracterul "letter"
    left = 0;
    right = this->wordListEntries.size() - 1;

    while (left <= right) {
        middle = left + (right - left) / 2;
        
        if (this->wordListEntries[middle].word[0] < letter) {
            left = middle + 1;
        } else if (this->wordListEntries[middle].word[0] > letter) {
            right = middle - 1;
        } else {
            // then: this->wordListEntries[middle].word[0] == letter)
            firstIndex = middle;
            right = middle - 1;
        }

    }


    if (firstIndex == -1) {
        return nullopt;
    }


    // Gasirea indexului ultimului cuvant care incepe cu caracterul "letter"
    left = 0;
    right = this->wordListEntries.size() - 1;

    while (left <= right) {
        middle = left + (right - left) / 2;

        if (wordListEntries[middle].word[0] < letter) {
            left = middle + 1;
        } else if (wordListEntries[middle].word[0] > letter) {
            right = middle - 1;
        } else {
            // then: wordListEntries[middle].word[0] == letter
            lastIndex = middle;
            left = middle + 1;
        }

    }


    if (lastIndex == -1) {
        return nullopt;
    }

    return WordListChunck(letter, firstIndex, lastIndex);
}




void WordList::writeInputFileIDs(ofstream &fout, set<int> &fileIDs)
{

    set<int>::iterator iter = fileIDs.begin(); 
    
    
    while (iter != fileIDs.end()) {
        fout << *iter;

        iter++;
        if (iter != fileIDs.end()) {
            fout << " ";
        }
    }
}


void WordList::writeWordListEntry(ofstream &fout, int &idx)
{
    WordListEntry &wordListEntry = this->wordListEntries[idx];

    fout << wordListEntry.word << ": [";
    writeInputFileIDs(fout, wordListEntry.fileIDs);
    fout << "]\n";
}


void WordList::writeLetterChunck(char letter)
{
    string outputFileName = string(1, letter) + ".txt";
    ofstream fout(outputFileName);

    if (!fout.is_open()) {
        cerr << "[ERROR] Cannot open input file <" << outputFileName << ">\n";
        return;
    }


    optional<WordListChunck> wordListChunck = findChunkOfCharacterInWordList(letter);

    if (wordListChunck) {
        for (int i = wordListChunck->firstIndex; i <= wordListChunck->lastIndex; i++) {
            writeWordListEntry(fout, i);
        }
    }

    fout.close();
}



void WordList::printWordList()
{
    cout << "Word List:\n";


    for (WordListEntry &elem : this->wordListEntries) {
        cout << elem.word << "-> ";
        for (int fileID : elem.fileIDs) {
            cout << fileID << ", ";
        }
        cout << "\n";
    }
    cout << "\n";
}







