#ifndef __EMBEDI_OPERATING_SYSTEM_HPP__
#define __EMBEDI_OPERATING_SYSTEM_HPP__

#include "IOStreams.hpp"
#include "CommandLine.hpp"
#include "DataStructures.hpp"

struct OperatingSystem
{
    EmbediFileStream* outputStream;
    EmbediFileStream* inputStream;
    EmbediFileStream* errorStream;
    FileTable fileTable;
    SystemVariables systemVariables;
    CommandLine terminal;
    EmbediString cwd;

    OperatingSystem() : cwd("/")
    {}
};

#endif