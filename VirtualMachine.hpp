#ifndef __EMBEDI_VIRTUAL_MACHINE_HPP__
#define __EMBEDI_VIRTUAL_MACHINE_HPP__

#include "TypeMacros.hpp"

/*
Supported Operations:

1) set <register> <number / pointer>
2) add <gpr1> <gpr2> -> <accumulator>
3) sub <gpr1> <gpr2> -> <accumulator>
4) div <gpr1> <gpr2> -> <accumulator>
5) mul <gpr1> <gpr2> -> <accumulator>
6) mod <gpr1> <gpr2> -> <accumulator>
*/

constexpr TypeMacros::u32 STACK_SIZE = Kb(1);
constexpr TypeMacros::u32 PROGRAM_SIZE = Kb(1);

struct VirtualMachine
{
    // Program
    TypeMacros::u8* PROGRAM = nullptr;

    // Program Counter
    TypeMacros::u64 PC = 0;

    // Accumulator
    TypeMacros::i64 ACC = 0;

    // Data
    TypeMacros::i64 DAT = 0;

    // General Purpose Registers
    TypeMacros::i64 R1 = 0;
    TypeMacros::i64 R2 = 0;
    TypeMacros::i64 R3 = 0;
    TypeMacros::i64 R4 = 0;
    TypeMacros::i64 R5 = 0;
    TypeMacros::i64 R6 = 0;
    TypeMacros::i64 R7 = 0;
    TypeMacros::i64 R8 = 0;
    TypeMacros::i64 R9 = 0;
    TypeMacros::i64 R10 = 0;
    TypeMacros::i64 R11 = 0;
    TypeMacros::i64 R12 = 0;

    // Stack
    TypeMacros::u8 STACK[STACK_SIZE];

    // Stack Pointer
    TypeMacros::u32 SP = 0;

    // This function resets the state of virtual machine
    void set(TypeMacros::u8* _program)
    {
        this->PROGRAM = _program;
        this->SP = 0;
        this->PC = 0;
        memset(this->STACK, 0, STACK_SIZE * sizeof(TypeMacros::u8));
    }
};

#endif