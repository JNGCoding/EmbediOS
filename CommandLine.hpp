#ifndef __EMBEDI_COMMAND_LINE_HPP__
#define __EMBEDI_COMMAND_LINE_HPP__

#include <stdint.h>
#include <string.h>

using CommandLineFunction = int(*)(int argc, const char* argv[]);

struct CommandLine
{
    // Function Storage
    // DLangHashtable<DLangString, CommandLineFunction> functions(LinearAllocator(), 10);

    // Error code
    enum
    {
        NO_ERROR,
        FUNCTION_NOT_FOUND,
        ERROR_ALLOCATION,
    } error_code = NO_ERROR;

    int latest_return_code = 0;

    void trigger(const char* command);
};

#endif