#include "interpreter.h"
#include <iostream>
#include <cstring>


struct Settings {
    Settings():
        lambdaAtBegin(false), comatAtEnd(false) {}

    std::string filename;
    bool lambdaAtBegin;
    bool comatAtEnd;
};


bool processArguments(int argc, char* argv[], Settings &arguments) {
    if (argc <= 1) {
        std::cout << "No input file specified. Process stopped." << std::endl;
        return false;
    }

    for (size_t i=1; i<argc; ++i) {
#ifdef WIN_32
        // todo: keys parsing here
        if (argv[i][0] == '/'){}
#endif

#ifdef LINUX
        // todo: keys parsing here
        if (argv[i] == "-l") {}
#endif

        else
            if (arguments.filename.empty())
                arguments.filename = argv[i];
            else
                std::cout << "WARNING: Additional filename \"" << argv[i] << "\" will be ignored. \""
                          << arguments.filename << "\" is used." << std::endl;
        }

    return true;
}


int main(int argc, char* argv[]) {
    Settings settings;
    if (! processArguments(argc, argv, settings))
        return 1;

    try {
        Interpreter interpreter;
        return interpreter.processFile(settings.filename);

    } catch (std::bad_alloc &) {
        std::cout << "ERROR: Not enough system memory. Process stopped.";
        return 1;
    } catch (std::exception &) {
        std::cout << "ERROR: Unknown error occured. Process stopped.";
        return 1;
    }
}

