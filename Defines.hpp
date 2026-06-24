#ifndef __EMBEDI_DEFINES_HPP__
#define __EMBEDI_DEFINES_HPP__

#include "TypeMacros.hpp"

constexpr const char* os_name = "EmbediOS";
constexpr const char* version = "0.00B";

// Max Commind Line Statement length
constexpr TypeMacros::u32 MAX_STATEMENT_LENGTH = 1024;

// MAX PROGRAM SIZE
constexpr TypeMacros::u32 MAX_PROGRAM_SIZE = Kb(10);

// Max Heap Size
constexpr TypeMacros::u32 MAX_HEAP_SIZE = Kb(0.5);

// SD Card SPI Chip Select Pin
constexpr TypeMacros::u8 SD_CARD_CHIP_SELECT = 5;


// SD Card is optional, but necessary for certain operations
// Some commands will be removed such as listdir and cd
#define COMPILE_WITH_SD_INITIALIZATION

#endif