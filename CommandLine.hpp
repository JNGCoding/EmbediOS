#ifndef __EMBEDI_COMMAND_LINE_HPP__
#define __EMBEDI_COMMAND_LINE_HPP__

// Main Command Line or Terminal program very special from other applications
// This will be responsible for launching all the other programs that will be stored
// on the SD Card
// This will have the utility to load them with correct IO Streams, Debugger, Handlers
// and launch them
// This will also prepare the Memory address or program space with which the program will
// be provided with special allocators to allocate memory
// To prevent Heap Fragmentation

#include <string.h>
#include "TypeMacros.hpp"
#include "Allocator.hpp"
#include "DataStructures.hpp"
#include "IOStreams.hpp"
#include "Application.hpp"
#include "SystemApplications.hpp"

// Number of arguments at max passable to an application
constexpr TypeMacros::u32 MAX_ARGS = 5;

// Max length of one arbitrary argument passable
// If any argument greater than this value is passed
// then the argument will be truncated
constexpr TypeMacros::u32 ARG_LENGTH = 64;

// MAX FUNCTIONS
constexpr TypeMacros::u32 MAX_FUNCTIONS = 128;

EmbediApplication* latestRanApplication = nullptr;

// I don't know how to give the Operating system the access to kill a process
// i.e, kill a program while it is running. I guess force the killing via a watchdog
// timer reset ? or IDK, I may have to implement the timer in software
// to not reset the whole operating system program
// The thing is, If I embed program runtime inside the operating system
// the program may be too slow
// I was thinking of implementing like ring 0 access like Terry Davis, I think his name is

inline bool is_space(const char ch) {
    return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\r' || ch == '\f' || ch == '\n' || ch == '\0';
}

// This function will read a file from a stream, such as SDCardFileStream
// and execute functions from that file by reading lines
static int command_set(int argc, const char* argv[])
{
    return 0;
}

static int echo(int argc, const char* argv[])
{
    if (argc >= 1)
        SystemIO::printf("%s\n", argv[0]);
    
    return 0;
}

struct CommandLine
{
    using CLFunction = int(*)(int argc, const char* argv[]);

    EmbediPair<const char*, CLFunction> functions[MAX_FUNCTIONS];
    TypeMacros::u32 appIndex = 0;

    CommandLine()
    {
        this->register_function("echo", echo);
    }

    bool register_function(const char* name, CLFunction func)
    {
        if (this->appIndex >= MAX_FUNCTIONS)
            return false;

        this->functions[this->appIndex++].set(name, func);
        return true;
    }

    struct FunctionPackage
    {
        CLFunction function;
        int argc;
        char argv_storage[MAX_ARGS][ARG_LENGTH];
        const char* argv[MAX_ARGS];
    } loadedFunction = {};

    FunctionPackage* process_statement(const char* statement)
    {
        TypeMacros::u32 strsize = strlen(statement);

        char name[64];

        TypeMacros::u32 i = 0;

        // Skip Leading spaces
        for (; is_space(statement[i]); i++);
        if (i >= strsize)
            return nullptr;

        // Process name (Compact code but all it does, is append characters to name)
        unsigned int nameAppIndex = 0;
        for (; i < strsize && !is_space(statement[i]); i++)
            name[nameAppIndex < 63 ? nameAppIndex++ : 62] = statement[i];
        name[nameAppIndex] = '\0';

        // Take the function
        bool flag = false;
        for (TypeMacros::u32 j = 0; j < this->appIndex; j++)
        {
            if (strcmp(this->functions[j].a, name) == 0)
            {
                this->loadedFunction.function = this->functions[j].b;
                flag = true;
                break;
            }
        }
        if (!flag)
            return nullptr;

        // Process Arguments
        unsigned int argIndex = 0;
        unsigned int argAppIndex = 0;

        while (i < strsize && argIndex < MAX_ARGS)
        {
            // Skip Leading spaces
            for (; is_space(statement[i]); i++);
            if (i >= strsize)
                break;

            // Read the characters
            if (statement[i] != '"')
            {
                for (; i < strsize && !is_space(statement[i]); i++)
                    this->loadedFunction.argv_storage[argIndex][argAppIndex < ARG_LENGTH - 1 ? argAppIndex++ : ARG_LENGTH - 2] = statement[i];
            }
            else
            {
                // Skip leading '"'
                i++;

                for (; i < strsize && statement[i] != '"'; i++)
                    this->loadedFunction.argv_storage[argIndex][argAppIndex < ARG_LENGTH - 1 ? argAppIndex++ : ARG_LENGTH - 2] = statement[i];

                // Skip trailing '"'
                i++;
            }

            this->loadedFunction.argv_storage[argIndex][argAppIndex] = '\0';
            this->loadedFunction.argv[argIndex] = this->loadedFunction.argv_storage[argIndex];

            argIndex++;
            argAppIndex = 0;
        }

        this->loadedFunction.argc = argIndex;

        return &this->loadedFunction;
    }
};

extern CommandLine CMD;

#endif