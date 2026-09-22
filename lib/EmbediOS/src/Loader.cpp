#include "OS_Setup.hpp"

#include "CommandLine/CommandLine.hpp"
#include "FileSystem/IOStreams.hpp"

OperatingSystem OS;

SystemVariables& __sysvars = OS.systemVariables;
FileTable& __filetable = OS.fileTable;