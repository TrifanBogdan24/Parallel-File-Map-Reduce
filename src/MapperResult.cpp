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

#include "MapperResult.h"

using namespace std;

MapperResultEntry::MapperResultEntry(const string& word_value, const int fileID_value)
        : word(word_value), fileID(fileID_value) {}

MapperResultEntry::~MapperResultEntry() = default;


MapperResult::MapperResult() = default;
MapperResult::~MapperResult() = default;

