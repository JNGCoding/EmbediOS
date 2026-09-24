#ifndef __EMBEDI_SYSTEM_VARIABLES_HPP__
#define __EMBEDI_SYSTEM_VARIABLES_HPP__

#include "Defines.hpp"
#include "../DataStructures/DataStructures.hpp"
#include "../FileSystem/IOStreams.hpp"

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
        variables(128, &basicAllocator)
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

    void print_all_entries(const char* stream_name)
    {
        EmbediFileStream* stream = __filetable.get_handle(stream_name);
        if (stream == nullptr)
        {
            SystemIO::perror("[SystemVariables::print_all_entries(EmbediFileStream*)] Error: stream provided is null\n");
            return;
        }

        for (int i = 0; i < 128; i++)
        {
            const EmbediList<EmbediPair<SystemVariableName, SystemVariableValue>>* bucket = this->variables.get_bucket(i);
            for (TypeMacros::u32 i = 0; i < bucket->size(); i++)
            {
                EmbediPair<SystemVariableName, SystemVariableValue>* entry = bucket->get(i);
                SystemIO::fprintf(stream, "%u) %s - %s\n", i, entry->a.name, entry->b.value);
            }
        }
    }

    void print_all_entries(EmbediFileStream* stream)
    {
        if (stream == nullptr)
        {
            SystemIO::perror("[SystemVariables::print_all_entries(EmbediFileStream*)] Error: stream provided is null\n");
            return;
        }

        for (int i = 0; i < 128; i++)
        {
            const EmbediList<EmbediPair<SystemVariableName, SystemVariableValue>>* bucket = this->variables.get_bucket(i);
            for (TypeMacros::u32 i = 0; i < bucket->size(); i++)
            {
                EmbediPair<SystemVariableName, SystemVariableValue>* entry = bucket->get(i);
                SystemIO::fprintf(stream, "%u) %s - %s\n", i, entry->a.name, entry->b.value);
            }
        }
    }
};

extern SystemVariables& __sysvars;

#endif