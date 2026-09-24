#ifndef __EMBEDI_OPERATING_SYSTEM_HPP__
#define __EMBEDI_OPERATING_SYSTEM_HPP__

#include "TypeMacros.hpp"
#include "Defines.hpp"

#include "FileSystem/IOStreams.hpp"
#include "CommandLine/CommandLine.hpp"
#include "CommandLine/SystemVariables.hpp"
#include "DataStructures/DataStructures.hpp"
#include "VirtualMachine/VirtualMachine.hpp"

struct OperatingSystem
{
    EmbediFileStream* outputStream = nullptr;
    EmbediFileStream* inputStream  = nullptr;
    EmbediFileStream* errorStream  = nullptr;
    FileTable fileTable;
    SystemVariables systemVariables;
    CommandLine terminal;
    EmbediString cwd;
    VirtualMachine VM;
    u8 program[ MAX_PROGRAM_SIZE ] = {0};

    OperatingSystem() : cwd("/")
    {}
};

extern OperatingSystem OS;

#endif