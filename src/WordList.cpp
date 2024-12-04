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
    int left, right, middle;

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
            // this->wordListEntries[middle].word[0] == letter
            firstIndex = middle;
            right = middle - 1;
        }
    }

    // Daca nu am gasit niciun cuvant cu litera cautata, iesim
    if (firstIndex == -1) {
        return nullopt;
    }

    // Gasirea indexului ultimului cuvant care incepe cu caracterul "letter"
    left = firstIndex;
    right = this->wordListEntries.size() - 1;
    while (left <= right) {
        middle = left + (right - left) / 2;

        if (this->wordListEntries[middle].word[0] < letter) {
            left = middle + 1;
        } else if (this->wordListEntries[middle].word[0] > letter) {
            right = middle - 1;
        } else {
            lastIndex = middle;
            left = middle + 1;
        }
    }

    return WordListChunck(letter, firstIndex, lastIndex);
}




void WordList::writeInputFileIDs(ostream &fout, set<int> &fileIDs)
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


void WordList::writeWordListEntry(ostream &fout, WordListEntry &wordListEntry)
{
    fout << wordListEntry.word << ":[";
    writeInputFileIDs(fout, wordListEntry.fileIDs);
    fout << "]\n";
}


bool WordList::compareEntries(const WordListEntry &entry1, const WordListEntry &entry2)
{
    if (entry1.fileIDs.size() != entry2.fileIDs.size()) {
        return entry1.fileIDs.size() > entry2.fileIDs.size();
    }
    return entry1.word < entry2.word;
}

void WordList::writeLetterChunck(char letter)
{
    string outputFileName = string(1, letter) + ".txt";


    ofstream fout(outputFileName);

    if (!fout.is_open()) {
        return;
    }


    optional<WordListChunck> wordListChunck = findChunkOfCharacterInWordList(letter);


    if (wordListChunck) {
        int firstIndex = wordListChunck->firstIndex;
        int lastIndex = wordListChunck->lastIndex;
        vector<WordListEntry> selectedEntries(
            this->wordListEntries.begin() + firstIndex,
            this->wordListEntries.begin() + lastIndex
        );

        sort(selectedEntries.begin(), selectedEntries.end(), compareEntries);


        for (WordListEntry &entry : selectedEntries) {
            writeWordListEntry(fout, entry);
        }
    }

    fout.close();
}



void WordList::printWordList()
{
    cout << "Word List:\n";

    for (WordListEntry &entry : this->wordListEntries) {
        writeWordListEntry(cout, entry);
        cout << "\n";
    }
}



void WordList::writeWordList()
{
    ofstream fout("word-list.txt");
    fout << "Word List:\n";

    for (WordListEntry &entry : this->wordListEntries) {
        writeWordListEntry(fout, entry);
        cout << "\n";
    }

    fout.close();
}





