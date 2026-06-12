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

#endif