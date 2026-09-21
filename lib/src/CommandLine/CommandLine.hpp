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

#include <Arduino.h>
#include <string.h>

#include "TypeMacros.hpp"
#include "Defines.hpp"

#include "Allocators/Allocator.hpp"
#include "DataStructures/DataStructures.hpp"
#include "FileSystem/IOStreams.hpp"

// I don't know how to give the Operating system the access to kill a process
// i.e, kill a program while it is running. I guess force the killing via a watchdog
// timer reset ? or IDK, I may have to implement the timer in software
// to not reset the whole operating system program
// The thing is, If I embed program runtime inside the operating system
// the program may be too slow
// I was thinking of implementing like ring 0 access like Terry Davis, I think his name is

// trivial data for storing system variables
// aka name - value, where both are strings
struct SystemVariableName { char name[MAX_SYSTEM_VAR_NAME_LENGTH]; };
struct SystemVariableValue { char value[ARG_LENGTH]; };

// Template specialization of HashTable Hash Function for SystemVariableName
template<>
inline TypeMacros::usize EMBEDI_GENERIC_HASH_FUNCTION<SystemVariableName>(SystemVariableName key)
{
    constexpr TypeMacros::usize FNV_offset_basis = 0xCBF29CE484222325;
    constexpr TypeMacros::usize FNV_prime = 1099511628211ULL;

    TypeMacros::usize hash = FNV_offset_basis;

    TypeMacros::u32 strsize = strlen(key.name);

    for (TypeMacros::u32 i = 0; i < strsize; i++)
    {
        hash = hash ^ key.name[i];
        hash = hash * FNV_prime;
    }

    return hash;
}

// Comparator function for Hashtable Comparison method
struct SVNComparator
{
    static bool equals(const SystemVariableName& a, const SystemVariableName& b)
    {
        return strcmp(a.name, b.name) == 0;
    }
};

struct SystemVariables
{
    EmbediHashTable<SystemVariableName, SystemVariableValue, SVNComparator> variables;

    SystemVariables() :
        variables(10, &basicAllocator)
    {}

    bool create(const char* name)
    {
        if (name == nullptr)
            return false;

        SystemVariableName varname;
        SystemVariableValue varval;
        snprintf(varname.name, MAX_SYSTEM_VAR_NAME_LENGTH, name);

        return this->variables.add(varname, varval);
    }

    bool create(const char* name, const char* value)
    {
        if (name == nullptr || value == nullptr)
            return false;

        SystemVariableName varname;
        snprintf(varname.name, MAX_SYSTEM_VAR_NAME_LENGTH, name);

        SystemVariableValue varval;
        snprintf(varval.value, ARG_LENGTH, value);

        return this->variables.add(varname, varval);
    }

    bool change(const char* name, const char* value)
    {
        if (name == nullptr || value == nullptr)
            return false;

        SystemVariableName varname;
        snprintf(varname.name, MAX_SYSTEM_VAR_NAME_LENGTH, name);

        SystemVariableValue* varval = this->variables.get(varname);
        if (varval == nullptr)
            return false;

        snprintf(varval->value, ARG_LENGTH, value);

        return true;
    }

    bool remove(const char* name)
    {
        if (name == nullptr)
            return false;

        SystemVariableName varname;
        snprintf(varname.name, MAX_SYSTEM_VAR_NAME_LENGTH, name);

        return this->variables.remove(varname);
    }

    SystemVariableValue* get_var(const char* name)
    {
        if (name == nullptr)
            return nullptr;

        SystemVariableName varname;
        snprintf(varname.name, MAX_SYSTEM_VAR_NAME_LENGTH, name);

        return this->variables.get(varname);
    }
};

extern SystemVariables sysVars;

struct CommandLine
{
    using CLFunction = int(*)(int argc, const char* argv[]);

    EmbediPair<const char*, CLFunction> functions[MAX_FUNCTIONS];
    TypeMacros::u32 appIndex = 0;

    // Registers a command string in the buffer
    // returns true if the process was successful else false
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

    // Parses a CMD statement
    // <function_name> <arg1> <arg2>...
    // Returns a FunctionPackage* for execution else nullptr
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

            // If we detect a system variable, then just replace the word with the variable's definition
            if (this->loadedFunction.argv_storage[argIndex][0] == '$')
            {
                SystemVariableValue* varval = sysVars.get_var(this->loadedFunction.argv_storage[argIndex] + 1);
                if (varval == nullptr)
                {
                    SystemIO::fprintf(SYSERR_NAME, "System Variable %s not defined\n", this->loadedFunction.argv_storage[argIndex] + 1);
                    return nullptr;
                }

                snprintf(this->loadedFunction.argv_storage[argIndex], ARG_LENGTH, varval->value);
            }

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