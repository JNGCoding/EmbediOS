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

EmbediApplication* latestRanApplication = nullptr;

// I don't know how to give the Operating system the access to kill a process
// i.e, kill a program while it is running. I guess force the killing via a watchdog
// timer reset ? or IDK, I may have to implement the timer in software
// to not reset the whole operating system program
// The thing is, If I embed program runtime inside the operating system
// the program may be too slow
// I was thinking of implementing like ring 0 access like Terry Davis, I think his name is

struct CommandLine
{
    using CLFunction = int(*)(int argc, const char* argv[]);
};

#endif