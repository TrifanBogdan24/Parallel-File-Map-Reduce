// C++ libraries
#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <vector>
#include <algorithm>

// C libraries
#include <pthread.h>

#include "WordList.h"
#include "MapperResult.h"


using namespace std;


void WordList::insertInMapperResultConcatenation(MapperResultEntry &mapperResultEntry)
{
    string word = mapperResultEntry.word;
    int fileID = mapperResultEntry.fileID;

    int idxFirstWordLetter = word[0] - 'a';

    if (this->mapperResultConcatenation[idxFirstWordLetter].find(word) == this->mapperResultConcatenation[idxFirstWordLetter].end()) {
        // Nu s-a gasit cuvantul in dictionars
        this->mapperResultConcatenation[idxFirstWordLetter][word] = {fileID};
        return;
    } else {
        this->mapperResultConcatenation[idxFirstWordLetter][word].insert(fileID);
        return;
    }
}


void WordList::createLetterChunck(int idxFirstWordLetter)
{
    for (const auto& [word, fileIDs] : mapperResultConcatenation[idxFirstWordLetter]) {
        this->wordListLetterChuncks[idxFirstWordLetter]
            .letterChunckEntries.push_back(WordListEntry(word, fileIDs));
    }
}


void WordList::sortLetterChunck(int idxFirstWordLetter)
{
    sort(
        this->wordListLetterChuncks[idxFirstWordLetter].letterChunckEntries.begin(),
        this->wordListLetterChuncks[idxFirstWordLetter].letterChunckEntries.end(),
        compareEntries
    );
}

bool WordList::compareEntries(const WordListEntry &entry1, const WordListEntry &entry2)
{
    // Pentru cuvintele care incep cu aceeasi litera,
    // descrescator dupa numarul de fisiere in care cuvantul apare
    if (entry1.fileIDs.size() != entry2.fileIDs.size()) {
        return (entry1.fileIDs.size() > entry2.fileIDs.size());
    }

    // La aceeasi litera cu care incep si acelasi numar de fisiere de intrare in care apar,
    // sortarea se va face alfabetic
    return (entry1.word < entry2.word);
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


void WordList::writeWordList(ostream& fout)
{
    fout << "Word List:\n";

    for (int i = 0; i < NUM_ALPHABET_LETTERS; i++) {
        for (WordListEntry &entry : this->wordListLetterChuncks[i].letterChunckEntries) {
            writeWordListEntry(fout, entry);
        }
    }
}


void WordList::printWordListToStdout()
{
    writeWordList(cout);
}



void WordList::writeWordListToFile()
{
    ofstream fout("word-list.txt");
    writeWordList(fout);
    fout.close();
}





void WordList::writeLetterChunck(int idxFirstWordLetter)
{
    char letter = 'a' + idxFirstWordLetter;
    string outputFileName = string(1, letter) + ".txt";


    ofstream fout(outputFileName);

    if (!fout.is_open()) {
        return;
    }

    for (WordListEntry &entry : this->wordListLetterChuncks[idxFirstWordLetter].letterChunckEntries) {
        writeWordListEntry(fout, entry);
    }

    fout.close();
}


