#ifndef __EMBEDI_DEFINES_HPP__
#define __EMBEDI_DEFINES_HPP__

#include "TypeMacros.hpp"

constexpr const char* OS_NAME = "EmbediOS";
constexpr const char* VERSION = "0.00B";

// Max Commind Line Statement length
constexpr TypeMacros::u32 MAX_STATEMENT_LENGTH = Kb(1);

// MAX PROGRAM SIZE
constexpr TypeMacros::u32 MAX_PROGRAM_SIZE = Kb(10);

// PROGRAM STACK SIZE
constexpr TypeMacros::u32 STACK_SIZE = Kb(1);

// SD Card SPI Chip Select Pin
constexpr TypeMacros::u8 SD_CARD_CHIP_SELECT = 5;

// OPERATIONAL MEMORY SIZE
constexpr TypeMacros::u32 OPERATIONAL_MEMORY_SIZE = Kb(1);

// PROGRAM DEFAULT HEAP SIZE
constexpr TypeMacros::u32 STARTING_HEAP_SIZE = 512;

// SD Card initialization
#define COMPILE_WITH_SD_INITIALIZATION
#ifndef COMPILE_WITH_SD_INITIALIZATION
#warning Compiling without SDCard initializations, Functions removed: lsdir, cd, start, dumpf, mkdir, rmdir, create, delete, editor
#endif

// Debug Functions
#define INCLUDE_DEBUG_FUNCTIONS
#ifndef INCLUDE_DEBUG_FUNCTIONS
#warning Compiling without debug functions, Functions removed: freemem
#endif

#endif