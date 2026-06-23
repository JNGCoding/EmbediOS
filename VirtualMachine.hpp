#ifndef __EMBEDI_VIRTUAL_MACHINE_HPP__
#define __EMBEDI_VIRTUAL_MACHINE_HPP__

#include "TypeMacros.hpp"
#include "IOStreams.hpp"

/*
Supported Operations:

1) mov <register> <number / pointer>
2) add <gpr1> <gpr2> -> <accumulator>
3) sub <gpr1> <gpr2> -> <accumulator>
4) div <gpr1> <gpr2> -> <accumulator>
5) mul <gpr1> <gpr2> -> <accumulator>
6) mod <gpr1> <gpr2> -> <accumulator>
*/

constexpr TypeMacros::u32 STACK_SIZE = Kb(1);
constexpr TypeMacros::u32 PROGRAM_SIZE = Kb(1);

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

    constexpr TypeMacros::u8 RegisterAhead         = 0;
    constexpr TypeMacros::u8 MemorySpotAhead       = 1;
    constexpr TypeMacros::u8 ImmediateIntegerAhead = 2;
    constexpr TypeMacros::u8 ImmediateFloatAhead   = 3;
}

namespace Syscalls
{
    constexpr int StdoutWrite = 1;
}

namespace VMErrors
{
    constexpr int PASS = 0;
    constexpr int HALTED = 1;
    constexpr int REGISTER_NOT_DEFINED = -1290;
    constexpr int INSTRUCTION_NOT_DEFINED = -2304;
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
    TypeMacros::u32 SP = 0;

    // Stack Base Pointer
    TypeMacros::u8* BP = &STACK;

    // Random Access Memory Block
    TypeMacros::u8* HEAP = nullptr;

    // Debuggers
    bool errorFlag = false;
    int errorCode = VMErrors::PASS;

    // Some basic helper functions
    TypeMacros::u8 read8()
    { return static_cast<TypeMacros::u8>(this->PROGRAM[this->PC++]); }
    TypeMacros::u16 read16()
    { return static_cast<TypeMacros::u16>(read8()) << 8 | static_cast<TypeMacros::u16>(read8()); }
    TypeMacros::u32 read32()
    { return static_cast<TypeMacros::u32>(read16()) << 16 | static_cast<TypeMacros::u32>(read16()); }

    // This function resets the state of virtual machine
    void load_program(TypeMacros::u8* _program, TypeMacros::u8 _progSize, TypeMacros::u8* memoryChunk);

    int start_program();

    void register_dump();
};

#endif