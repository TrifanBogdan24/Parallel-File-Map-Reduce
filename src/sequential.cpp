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

using namespace std;


struct MapperElement {
    string word;
    int fileID;        

    MapperElement(const string& wordValue, const int fileID_Value)
        : word(wordValue), fileID(fileID_Value) {}
};

using MapperResult = vector<MapperElement>;


struct WordListElement {
    string word;
    set<int> fileIDs;

    WordListElement(const string &wordValue) :
        word(wordValue)
    {
    }

    WordListElement(const string &wordValue, const set<int> &fileIDValues):
        word(wordValue), fileIDs(fileIDValues)
    {
    }


    ~WordListElement()
    {
    }
};

using WordList = vector<WordListElement>;



int chars_to_int(char *str)
{
    if (str == NULL) {
        return -1;
    }

    int num = 0;

    for (char *ptr = str; *ptr != '\0'; ptr++) {
        if (*ptr < '0' || *ptr > '9') {
            return 1;
        }
        num = num * 10 + (*ptr - '0');
    }

    return num;
}

void validate_input(int argc, char* argv[])
{
    if (argc != 4) {
        cerr << "Invalid number of arguments!\n";
        cerr << "The program expects exactly 3 arguments in the command line!\n";
        cerr << "./exe <numar_mapperi> <numar_reduceri> <fisier_intrare>\n";
        exit(EXIT_FAILURE);
    }


    bool isValidInput = true;

    int numMappers = chars_to_int(argv[1]);
    int numReducers = chars_to_int(argv[2]);


    if (numMappers <= 0) {
        cerr << "[ERROR] The frist argument in invalid!\n";
        cerr << "The first argument (number of mapperResults) is expected to be a positive non-zero number.\n";
        isValidInput = false;
    }

    if (numReducers <= 0) {
        cerr << "[ERROR] The frist argument in invalid!\n";
        cerr << "The first argument (number of mapperResults) is expected to be a positive non-zero number.\n";
        isValidInput = false;
    }


    if (!isValidInput) {
        exit(EXIT_FAILURE);
    }
}


vector<string> read_inptut_file(string inputFileName)
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





set<string> get_unique_words_in_file(string &inputFileName)
{
    ifstream fin(inputFileName);


    if (!fin.is_open()) {
        cerr << "[ERROR] Cannot open input file <" << inputFileName << ">\n";
        // Return an empty set
        return set<string>();
    }

    set<string> uniqueWords;


    string line;

    while (getline(fin, line)) {
        for (char& chr : line) {
            chr = isalpha(chr) ? tolower(chr) : ' ';
        }

        char* str = strtok(&line[0], " \t\n");

        while (str != NULL) {
            string word(str);

            if (!word.empty()) {
                uniqueWords.insert(word);
            }

            str = strtok(NULL, " \t\n");
        }

    }


    fin.close();
    return uniqueWords;
}




// ordinea alfabetica
int compare_words(string &word1, string &word2)
{
    if (word1 < word2) {
        // "-1" daca word1 inaintea lui word2 in ordinea lexicografica
        return -1;
    } else if (word1 > word2) {
        // "1" daca word1 dupa word2 in ordinea lexicografica
        return 1;
    }
    return 0;
}


void insert_in_word_list(WordList* wordList, MapperElement &mapperElement)
{
    string word = mapperElement.word;
    int fileID = mapperElement.fileID;

    int length = wordList->size();

    if (length == 0) {
        wordList->push_back(WordListElement(word, {fileID}));
        return;
    }

    if (word < wordList->at(0).word) {
        wordList->insert(wordList->begin(), WordListElement(word, {fileID}));
        return;
    }

    if (word > wordList->at(length - 1).word) {
        wordList->push_back(WordListElement(word, {fileID}));
        return;
    }


    int left = 0;
    int right = length - 1;
    int middle = 0;

    while (left <= right) {
        middle = left + (right - left) / 2;


        if (wordList->at(middle).word == word) {
            // Updating word's set
            wordList->at(middle).fileIDs.insert(fileID);
            return;
        }

        if (wordList->at(middle).word < word) {
            left = middle + 1;
        } else {
            right = middle - 1;
        }
    }

}






void print_map_results(vector<MapperResult> &mapperResults)
{
    for (unsigned int i = 0; i < mapperResults.size(); i++) {
        cout << "Mapper " << i << ":\n";
        for (MapperElement &pair : mapperResults[i]) {
            cout << "{ " << pair.word << ", " << pair.fileID << " }\n";
        }
        cout << "\n";
    }
}



void print_wordList(WordList &wordList)
{
    cout << "Word List:\n";

    for (WordListElement &elem : wordList) {
        cout << elem.word << "-> ";
        for (int fileID : elem.fileIDs) {
            cout << fileID << ", ";
        }
        cout << "\n";
    }
    cout << "\n";
}




void sequential_compute_mapper_results(vector<string> &inputFileNames,  vector<MapperResult> &mapperResults)
{
    mapperResults.resize(inputFileNames.size());

    for (unsigned int i = 0; i < inputFileNames.size(); i++) {
        string inputFileName = inputFileNames[i];
        set<string> uniqueWords = get_unique_words_in_file(inputFileName);

        
        for (set<string>::iterator itr = uniqueWords.begin(); itr != uniqueWords.end(); itr++) {
            string word = *itr;
            int file_ID = i;
            
            mapperResults[i].push_back(MapperElement(word, file_ID));
        }
    }
}


void write_file_IDs(ofstream &fout, vector<int> &fileIDs)
{

    for (unsigned int i = 0; i < fileIDs.size(); i++) {
        fout << fileIDs[i];
        if (i != fileIDs.size() - 1) {
            fout << " ";
        }
    }
}


void write_word_file_ID(ofstream &fout, WordList &wordList, unsigned int &idx)
{
    fout << wordList[idx].word << ": [";
    
    vector<int> fileIDs(wordList[idx].fileIDs.begin(), wordList[idx].fileIDs.end());
    sort(fileIDs.begin(), fileIDs.end());

    write_file_IDs(fout, fileIDs);
    fout << "]\n";
}


void sequential_write_reducer_results(WordList &wordList)
{
    unsigned int idx = 0;

    for (char ch = 'a'; ch <= 'z'; ch++) {
        string outputFileName = string(1, ch) + ".txt";
        ofstream fout(outputFileName);

        if (!fout.is_open()) {
            cerr << "[ERROR] Cannot create file <" << outputFileName << ">\n";
            continue;
        }

        while(idx < wordList.size() && wordList[idx].word[0] == ch) {
            write_word_file_ID(fout, wordList, idx);
            idx++;
        }

        fout.close();
    }
}


int main(int argc, char* argv[])
{
    string inputFileName = argv[3];

    vector<string> inputFileNames = read_inptut_file(inputFileName);


    

    // cate un mapper pentru fiecare fisier
    vector<MapperResult> mapperResults;
    sequential_compute_mapper_results(inputFileNames, mapperResults);


    // un singur reducer
    WordList wordList;

    for (MapperResult &mapperResult : mapperResults) {
        for (MapperElement &elem : mapperResult) {
            insert_in_word_list(&wordList, elem);
        }
    }



    print_map_results(mapperResults);
    print_wordList(wordList);
    
    sequential_write_reducer_results(wordList);


    return 0;
}