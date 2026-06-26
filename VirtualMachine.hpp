#ifndef __EMBEDI_VIRTUAL_MACHINE_HPP__
#define __EMBEDI_VIRTUAL_MACHINE_HPP__

#include "Allocator.hpp"
#include "TypeMacros.hpp"
#include "IOStreams.hpp"

constexpr TypeMacros::u32 STACK_SIZE = Kb(1);

constexpr TypeMacros::u8 INTEGER_REGISTER_SPACE = 16;
constexpr TypeMacros::u8 FLOAT_REGISTER_SPACE = 16;
constexpr TypeMacros::u8 INTEGER_OFFSET = 1;
constexpr TypeMacros::u8 FLOAT_OFFSET = 20;

namespace Instructions
{
    constexpr TypeMacros::u8 nop      = 0;
    constexpr TypeMacros::u8 set      = 1;
    constexpr TypeMacros::u8 read     = 2;
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
    // heap extend instruction
    // write memory instruction
    // read memory instruction

    constexpr TypeMacros::u8 RegisterAhead         = 0;
    constexpr TypeMacros::u8 MemorySpotAhead       = 1;
    constexpr TypeMacros::u8 ImmediateIntegerAhead = 2;
    constexpr TypeMacros::u8 ImmediateFloatAhead   = 3;
}

namespace Syscalls
{
    constexpr TypeMacros::i32 StdoutWrite = 1;
}

namespace VMErrors
{
    constexpr int PASS = 0;
    constexpr int HALTED = 1;
    constexpr int REGISTER_NOT_DEFINED = -1290;
    constexpr int INSTRUCTION_NOT_DEFINED = -2304;
    constexpr int INVALID_STACK_OFFSET = -1223;
    constexpr int STACK_OVERFLOW = -2901;
    constexpr int STACK_UNDERFLOW = -1297;
    constexpr int INVALID_HEAP_ADDRESS =-12974;
};

struct VirtualMachine
{
    // Program
    TypeMacros::u8* PROGRAM = nullptr;
    TypeMacros::u32 progSize = 0;

    // Program Counter
    TypeMacros::u32 PC = 0;

    // Accumulator Register
    TypeMacros::i32 ACC = 0;

    // Data Register
    TypeMacros::i32 DAT = 0;

    // Instruction Register
    TypeMacros::u8 IR = 0;

    // Syscall Register
    TypeMacros::i32 SR = 0;

    // Float mode flag
    bool floatFlag = false;

    // General Purpose Registers
    // Integer GPRs
    TypeMacros::i32 IREGS[INTEGER_REGISTER_SPACE];
    // Float GPRs
    TypeMacros::f64 FREGS[FLOAT_REGISTER_SPACE];

    // Stack
    TypeMacros::u8 STACK[STACK_SIZE];

    // Stack Pointer
    TypeMacros::u8* SP = STACK;

    // Stack offset, for bounds checking
    TypeMacros::u32 SP_OFFSET = 0;

    // Random Access Memory Block
    TypeMacros::u8* HEAP = nullptr;
    TypeMacros::u32 heapSize = 0;
    EmbediAllocator* allocator = nullptr;

    // Debuggers
    bool errorFlag = false;
    int errorCode = VMErrors::PASS;

    // Streams
    EmbediFileStream* outStream = nullptr;
    EmbediFileStream* inStream  = nullptr;

    // Some basic helper functions
    TypeMacros::u8 read8()
    { return static_cast<TypeMacros::u8>(this->PROGRAM[this->PC++]); }
    TypeMacros::u16 read16()
    { return static_cast<TypeMacros::u16>(read8()) << 8 | static_cast<TypeMacros::u16>(read8()); }
    TypeMacros::u32 read32()
    { return static_cast<TypeMacros::u32>(read16()) << 16 | static_cast<TypeMacros::u32>(read16()); }

    // Setups the Input and Output streams provided by the user
    // And uses them in the write and read operations of the VM
    void load_streams(EmbediFileStream* out, EmbediFileStream* in);

    // Resets the state of the Virtual Machine and Loads the program in the VM
    void load_program(TypeMacros::u8* _program, TypeMacros::u32 _progSize, EmbediAllocator* _allocator);

    // Starts the processing and execution of the program
    // All the Return codes are stored in VMErrors Namespace
    // If result is not 0 or 1 (RETURN UPON HALT INSTRUCTION)
    // Then you may check the error code with VMErrors
    // warning: call clear_heap() after this function, to free all the ram allocated by this instance
    int start_program();

    // Clears all the heap memory allocated by the VM's allocator
    void clear_heap();

    // Dumps the content of the registers, in the SYSOUT stream
    void register_dump();
};

#endif