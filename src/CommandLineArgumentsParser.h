#include <string>

#include <stdlib.h>
#include <cstring>
#include <cctype>

using namespace std;

class CommandLineArgumentsParser {
 private:
    int numMappers;
    int numReducers;
    string inputFileName;

 private:
    int argc;
    char** argv;

 public:
    CommandLineArgumentsParser(int argc, char* argv[]);
    ~CommandLineArgumentsParser();

 private:
    int charsToInt(char *str);
    // If the CLI arguments are not valid, it will FORCE EXIT the program
    void validateInput();

 public:
    int getNumMappers();
    int getNumReducers();
    string getInputFileName();
};