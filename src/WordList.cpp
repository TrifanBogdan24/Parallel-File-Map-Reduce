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

#include "WordList.h"
#include "MapperResult.h"


using namespace std;


void WordList::insertInMapperResultConcatenation(MapperResultEntry &mapperResultEntry)
{
    string word = mapperResultEntry.word;
    int fileID = mapperResultEntry.fileID;

    if (this->mapperResultConcatenation.find(word) != this->mapperResultConcatenation.end()) {
        this->mapperResultConcatenation[word] = {fileID};
        return;
    } else {
        this->mapperResultConcatenation[word].insert(fileID);
        return;
    }
}



void WordList::createWordListFromMapperResultConcatenation()
{
    this->wordListEntries.clear();
    for (const auto& [word, fileIDs] : mapperResultConcatenation) {
        wordListEntries.push_back(WordListEntry(word, fileIDs));
    }

    std::sort(wordListEntries.begin(), wordListEntries.end(), [](const WordListEntry& a, const WordListEntry& b) {
        return a.word < b.word;
    });
}



WordListChunck WordList::findChunkOfCharacterInWordList(char letter)
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

    // Daca nu am gasit niciun cuvant cu litera cautata, iesim din functie
    if (firstIndex == -1) {
        return WordListChunck(letter, -1, -1);
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


    WordListChunck wordListChunck = findChunkOfCharacterInWordList(letter);
    int firstIndex = wordListChunck.firstIndex;
    int lastIndex = wordListChunck.lastIndex;

    if (firstIndex < 0 || lastIndex < 0) {
        fout.close();
        return;
    }


    vector<WordListEntry> selectedEntries(
        this->wordListEntries.begin() + firstIndex,
        this->wordListEntries.begin() + lastIndex + 1
    );

    sort(selectedEntries.begin(), selectedEntries.end(), compareEntries);


    for (WordListEntry &entry : selectedEntries) {
        writeWordListEntry(fout, entry);
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





