#ifndef __EMBEDI_COMMAND_LINE_HPP__
#define __EMBEDI_COMMAND_LINE_HPP__

#include <stdint.h>
#include <string.h>

using CommandLineFunction = int(*)(int argc, const char* argv[]);

struct CommandLine
{
    struct
    {
        int argc;
        char* argv[];
        CommandLineFunction function;
    } LoadedFunction;

    // Function Storage
    bool load_function(const char* command);
    int run_function();
};

#endif