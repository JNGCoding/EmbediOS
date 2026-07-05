#ifndef __EMBEDI_VIRTUAL_MACHINE_HPP__
#define __EMBEDI_VIRTUAL_MACHINE_HPP__

#include <Arduino.h>
#include "Allocator.hpp"
#include "TypeMacros.hpp"
#include "IOStreams.hpp"
#include "Defines.hpp"

constexpr TypeMacros::u8 INTEGER_REGISTER_SPACE = 16;
constexpr TypeMacros::u8 FLOAT_REGISTER_SPACE   = 16;
constexpr TypeMacros::u8 INTEGER_OFFSET         = 1;
constexpr TypeMacros::u8 FLOAT_OFFSET           = 20;

namespace Instructions
{
    constexpr TypeMacros::u8 noop     = 0;
    constexpr TypeMacros::u8 set      = 1;
    constexpr TypeMacros::u8 ujmp     = 3;
    constexpr TypeMacros::u8 jmpi0    = 4;
    constexpr TypeMacros::u8 jmpip    = 5;
    constexpr TypeMacros::u8 jmpin    = 6;
    constexpr TypeMacros::u8 vadd     = 7;
    constexpr TypeMacros::u8 vsub     = 8;
    constexpr TypeMacros::u8 vmul     = 9;
    constexpr TypeMacros::u8 vdiv     = 10;
    constexpr TypeMacros::u8 vmod     = 11;
    constexpr TypeMacros::u8 vrshift  = 12;
    constexpr TypeMacros::u8 vlshift  = 13;
    constexpr TypeMacros::u8 vand     = 14;
    constexpr TypeMacros::u8 vor      = 15;
    constexpr TypeMacros::u8 vnot     = 16;
    constexpr TypeMacros::u8 push     = 17;
    constexpr TypeMacros::u8 pop      = 18;
    constexpr TypeMacros::u8 syscall  = 19;
    constexpr TypeMacros::u8 halt     = 20;
    constexpr TypeMacros::u8 vxor     = 21;
    constexpr TypeMacros::u8 vinc     = 22;
    constexpr TypeMacros::u8 vdec     = 23;
    constexpr TypeMacros::u8 hextend  = 24;
    constexpr TypeMacros::u8 write    = 25;
    constexpr TypeMacros::u8 read     = 26;
    constexpr TypeMacros::u8 uwrite   = 27;
    constexpr TypeMacros::u8 uread    = 28;

    constexpr TypeMacros::u8 RegisterAhead         = 0;
    constexpr TypeMacros::u8 MemorySpotAhead       = 1;
    constexpr TypeMacros::u8 ImmediateIntegerAhead = 2;
    constexpr TypeMacros::u8 ImmediateFloatAhead   = 3;
}

namespace Syscalls
{
    constexpr TypeMacros::i32 StdoutWrite = 1;
    constexpr TypeMacros::i32 PinModeSet  = 2;
    constexpr TypeMacros::i32 PinSet      = 3;
    constexpr TypeMacros::i32 PinRead     = 4;
    constexpr TypeMacros::i32 CMDRun      = 5;
}

namespace VMErrors
{
    constexpr int PASS = 0;
    constexpr int HALTED = 1;
    constexpr int REGISTER_NOT_DEFINED    = -1;
    constexpr int INSTRUCTION_NOT_DEFINED = -2;
    constexpr int INVALID_STACK_OFFSET    = -3;
    constexpr int STACK_OVERFLOW          = -4;
    constexpr int STACK_UNDERFLOW         = -5;
    constexpr int INVALID_HEAP_ADDRESS    = -6;
    constexpr int ALLOCATOR_NOT_DEFINED   = -7;
    constexpr int ALLOCATION_FAILED       = -8;
    constexpr int INVALID_MEMORY_ADDRESS  = -9;
    constexpr int UNSAFE_MODE_NOT_ENABLED = -10;
    constexpr int INVALID_HEAP_OFFSET     = -11;
    constexpr int FEATURE_NOT_IMPLEMENTED = -12;
};

// VMWord depends on the architecture of the machine, since I am using ESP32 which follows a 32-bit architecture
// I am using 32 bits words, VMWord has to be set according to the memory space of the machine
// On AVR Micro-controllers, VMWord will be 16-bits in order to store 12-bits of memory pointers
using VMWord = TypeMacros::i32;

// This is constant alias, doesn't change from machine to machine
// since Program memory is processed and addressed in singular bytes
using VMPtr  = TypeMacros::u8*;

// to enable random memory access ?? could have used it for everything but doesn't follow
// signed conventions, so IDK why I created it to be honest
using VMRPtr = TypeMacros::uintptr;

struct VirtualMachineV2
{
    // Memory chunk
    TypeMacros::u8* PROGRAM_MEMORY = nullptr;
    TypeMacros::u32 programSize = 0;

    TypeMacros::u32 chunkSize = 0;

    // Program Counter
    TypeMacros::u32 PC = 0;

    // Accumulator Register
    VMWord ACC = 0;

    // Addressing Register (Enables Random access to memory)
    VMRPtr ADDR = 0;

    // Data Register
    VMWord DAT = 0;

    // Instruction Register
    TypeMacros::u8 IR = 0;

    // Syscall Register
    VMWord SR = 0;

    // Integer GPRs
    VMWord IREGS[INTEGER_REGISTER_SPACE];

    // Float GPRs
    TypeMacros::f64 FREGS[FLOAT_REGISTER_SPACE];

    // Program Stack
    VMPtr STACK = nullptr;

    // Stack Pointer (an offset variable)
    VMPtr STACK_POINTER = nullptr;

    // Heap Pointer (done for addressing in heap section of the program)
    VMPtr HEAP_POINTER = nullptr;

    // Region Specifiers, they are special pointers
    // Don't change them after setting up, they will be constant and will always be
    // pointing to the start of their respective regions
    // helpful to calculate pointer offsets
    VMPtr STACK_START_POINTER = nullptr;
    VMPtr HEAP_START_POINTER  = nullptr;

    // Memory Allocator
    EmbediAllocator* vmallocator = nullptr;

    // error debuggers
    TypeMacros::u8 ERROR_CODE = VMErrors::PASS;

    // extreme unsafe mode
    bool UNSAFE_MODE = false;

    // I/O Streams
    EmbediFileStream* vmout;
    EmbediFileStream* vmin;

    constexpr bool STACK_OVERFLOW_CHECK(TypeMacros::u32 delta)
    { return this->STACK_POINTER + delta >= programSize + STACK_SIZE + this->PROGRAM_MEMORY; }

    constexpr bool STACK_UNDERFLOW_CHECK(TypeMacros::u32 delta)
    { return this->STACK_POINTER - delta < programSize + this->PROGRAM_MEMORY; }

    constexpr bool INVALID_HEAP_ADDRESS_CHECK(TypeMacros::u32 delta)
    { return this->HEAP_POINTER + delta > chunkSize + this->PROGRAM_MEMORY || this->HEAP_POINTER + delta < this->PROGRAM_MEMORY + STACK_SIZE; }

    // Helper functions
    TypeMacros::u8 read8()
    { return static_cast<TypeMacros::u8>(this->PROGRAM_MEMORY[this->PC++]); }
    TypeMacros::u16 read16()
    { return static_cast<TypeMacros::u16>(read8()) << 8 | static_cast<TypeMacros::u16>(read8()); }
    TypeMacros::u32 read32()
    { return static_cast<TypeMacros::u32>(read16()) << 16 | static_cast<TypeMacros::u32>(read16()); }

    // This function is a must for running a program
    // It prepares the buffers, allocates the PROGRAM_MEMORY on the heap
    // Resets the state of the Virtual Machine V2
    // and sets up the Stack and Heap pointers
    bool prepare(
        EmbediFileStream* out,
        EmbediFileStream* in,
        EmbediAllocator* allocator,
        TypeMacros::u8* program,
        TypeMacros::u32 progSize,
        TypeMacros::u32 startingHeapSize
    );

    // Starts the processing and execution of the program
    // All the Return codes are stored in VMErrors Namespace
    // If result is not 0 or 1 (RETURN UPON HALT INSTRUCTION)
    // Then you may check the error code with VMErrors
    // warning: call cleanup() after this function, to free all the ram allocated by this instance
    int start_program();

    // Calls start_program, then cleanup
    int run();

    // Clears all the Program Heap memory, and pointer variables. Must be called after start_program
    void cleanup();

    // Prints the register state of the Virtual Machine, onto the Standard output
    void register_dump();
};

// Fixes the main error in the V1 Virtual Machine
// All the stack memory, Heap memory and instruction memory now lie in the
// unified memory chunk called program memory
// This solves the horrible memory addressing in the Virtual Machine V1
// In the later builds, V1 will be completed deleted

#endif