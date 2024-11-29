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

using namespace std;

class MapperResultEntry {
 public:
    string word;
    int fileID;

 public:
    MapperResultEntry(const string& word_value, const int fileID_value);
    ~MapperResultEntry();
};


class MapperResult {
 public:
    vector<MapperResultEntry> mapperResultEntries;

 public:
    MapperResult();
    ~MapperResult();

};
