#pragma once

// C++ libraries
#include <string>
#include <vector>


using namespace std;

class MapperResultEntry {
 public:
    string word;
    int fileID;

 public:
    MapperResultEntry(const string& word_value, const int fileID_value)
      : word(word_value), fileID(fileID_value) {};
    ~MapperResultEntry() = default;
};


class MapperResult {
 public:
    vector<MapperResultEntry> mapperResultEntries;

 public:
    MapperResult() = default;
    ~MapperResult() = default;

};
