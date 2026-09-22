#ifndef __EMBEDI_OS_OPERATING_SYSTEM_HPP__
#define __EMBEDI_OS_OPERATING_SYSTEM_HPP__

#include "TypeMacros.hpp"
#include "Defines.hpp"

#include "FileSystem/IOStreams.hpp"
#include "CommandLine/CommandLine.hpp"
#include "DataStructures/DataStructures.hpp"
#include "VirtualMachine/VirtualMachine.hpp"

struct OperatingSystem
{
    EmbediFileStream* inputStream  = nullptr;
    EmbediFileStream* outputStream = nullptr;
    EmbediFileStream* errorStream  = nullptr;

    EmbediString cwd;

    FileTable fileTable;

    SystemVariables systemVariables;
    CommandLine terminal;

    VirtualMachine VM;
    u8 programMem[MAX_PROGRAM_SIZE] = {0};

    OperatingSystem() : fileTable(), terminal(), cwd("/")
    {}
};

#endif