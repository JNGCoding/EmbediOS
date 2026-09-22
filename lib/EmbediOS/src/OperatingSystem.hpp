#ifndef __EMBEDI_OPERATING_SYSTEM_HPP__
#define __EMBEDI_OPERATING_SYSTEM_HPP__

#include "TypeMacros.hpp"
#include "Defines.hpp"
#include "IOStreams.hpp"
#include "CommandLine.hpp"
#include "DataStructures.hpp"
#include "VirtualMachine.hpp"

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