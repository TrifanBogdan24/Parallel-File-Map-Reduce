#include <iostream>
#include <string>


#include "CommandLineArgumentsParser.h"

using namespace std;


CommandLineArgumentsParser::CommandLineArgumentsParser(int argcValue, char* argvValues[])
{
    this->argc = argcValue;
    this->argv = argvValues;

    validateInput();

}

CommandLineArgumentsParser::~CommandLineArgumentsParser() = default;


int CommandLineArgumentsParser::getNumMappers()
{
    return this->numMappers;     // argv[1]
}


int CommandLineArgumentsParser::getNumReducers()
{
    return this->numReducers;    // argv[2]
}

string CommandLineArgumentsParser::getInputFileName()
{
    return this->inputFileName;  // argv[3]
}


int CommandLineArgumentsParser::charsToInt(char* str)
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


void CommandLineArgumentsParser::validateInput()
{
    if (argc != 4) {
        cerr << "Invalid number of arguments!\n";
        cerr << "The program expects exactly 3 arguments in the command line!\n";
        cerr << "./exe <numar_mapperi> <numar_reduceri> <fisier_intrare>\n";
        exit(EXIT_FAILURE);
    }


    bool isValidInput = true;

    this->numMappers = charsToInt(argv[1]);
    this->numReducers = charsToInt(argv[2]);
    this->inputFileName = argv[3];

    if (this->numMappers <= 0) {
        cerr << "[ERROR] The frist argument in invalid!\n";
        cerr << "The first argument (number of mapperResults) is expected to be a positive non-zero number.\n";
        isValidInput = false;
    }

    if (this->numReducers <= 0) {
        cerr << "[ERROR] The frist argument in invalid!\n";
        cerr << "The first argument (number of mapperResults) is expected to be a positive non-zero number.\n";
        isValidInput = false;
    }


    if (!isValidInput) {
        exit(EXIT_FAILURE);
    }
}


