#include <string>
#include <iostream>

using namespace std;


int charsToInt(char* str)
{
    if (str == NULL) {
        return -1;
    }

    int num = 0;

    for (char *ptr = str; *ptr != '\0'; ptr++) {
        if (*ptr < '0' || *ptr > '9') {
            return -1;
        }
        num = num * 10 + (*ptr - '0');
    }

    return num;
}


bool validateInput(int argc, char* argv[])
{
    if (argc != 4) {
        cerr << "Invalid number of arguments!\n";
        cerr << "The program expects exactly 3 arguments in the command line!\n";
        cerr << "./exe <numar_mapperi> <numar_reduceri> <fisier_intrare>\n";
        exit(EXIT_FAILURE);
    }


    bool isValidInput = true;

    int numMappers = charsToInt(argv[1]);
    int numReducers = charsToInt(argv[2]);
    string inputFileName = argv[3];

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


    return isValidInput;
}
