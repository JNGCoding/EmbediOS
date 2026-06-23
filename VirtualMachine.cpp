#include "VirtualMachine.hpp"

void VirtualMachine::load_program(TypeMacros::u8 *_program, TypeMacros::u8 _progSize, TypeMacros::u8 *memoryChunk)
{
    this->PROGRAM = _program;
    this->progSize = _progSize;
    this->SP = 0;
    this->PC = 0;
    this->SR = 0;
    memset(this->STACK, 0, STACK_SIZE * sizeof(TypeMacros::u8));
    this->HEAP = memoryChunk;
}

inline void set_register(VirtualMachine& vm, const TypeMacros::u8 r, const TypeMacros::i32 i, const TypeMacros::f64 f)
{
    if (r >= INTEGER_OFFSET && r <= (INTEGER_OFFSET + INTEGER_REGISTER_SPACE))
    {
        vm.IREGS[r - INTEGER_OFFSET] = i;
        return;
    }

    if (r >= FLOAT_OFFSET && r <= (FLOAT_OFFSET + FLOAT_REGISTER_SPACE))
    {
        vm.FREGS[r - FLOAT_OFFSET] = f;
        return;
    }

    if (r == 255)
    {
        vm.ACC = i;
    }
    else if (r == 254)
    {
        vm.DAT = i;
    }
    else if (r == 253)
    {
        vm.SR = i;
    }
    else
    {
        SystemIO::printf("Register definition not found\n");
        vm.errorFlag = true;
        vm.errorCode = VMErrors::REGISTER_NOT_DEFINED;
    }
}

inline TypeMacros::f64 get_register(VirtualMachine& vm, const TypeMacros::u8 r) {
    if (r >= 1 && r <= (r + INTEGER_REGISTER_SPACE))
        return vm.IREGS[r - 1];

    if (r >= 20 && r <= (r + FLOAT_REGISTER_SPACE))
        return vm.FREGS[r - 20];

    if (r == 255)
    {
        return vm.ACC;
    }
    else if (r == 254)
    {
        return vm.DAT;
    }
    else if (r == 253)
    {
        return vm.SR;
    }
    else
    {
        SystemIO::printf("Register definition not found\n");
        vm.errorFlag = true;
        vm.errorCode = VMErrors::REGISTER_NOT_DEFINED;
        return -1;
    }
}

// Okay thought of a method, to indicate whether I want to write
// to heap or the stack, simple

// I don't fucking know, what to do ???
// usually, I should have used a value container arrangement
// so a mov instruction, so maybe I will do just that
// because data is easier to process that way
// Should implement that in assembler
int VirtualMachine::start_program()
{
    // IT FUCKIN WORKED BITCHES
    while (this->PC < this->progSize)
    {
        if (this->errorFlag)
            return this->errorCode;
        
        this->IR = this->PROGRAM[this->PC++];

        if (this->IR == Instructions::set)
        {
            TypeMacros::u8 valueFlag = this->PROGRAM[this->PC++];
            TypeMacros::f64 value;

            if (valueFlag == Instructions::ImmediateIntegerAhead)
            {
                TypeMacros::i32 local = this->read32();
                value = static_cast<TypeMacros::f64>(local);
            }
            else if (valueFlag == Instructions::MemorySpotAhead)
            {
                TypeMacros::u32 address = this->read32();
                value = static_cast<TypeMacros::f64>(this->HEAP[address]);
            }
            else if (valueFlag == Instructions::RegisterAhead)
            {
                value = get_register(*this, this->PROGRAM[this->PC++]);
            }

            TypeMacros::u8 containerFlag = this->PROGRAM[this->PC++];

            if (containerFlag == Instructions::RegisterAhead)
            {
                TypeMacros::u8 r = this->PROGRAM[this->PC++];
                set_register(*this, r, static_cast<TypeMacros::i32>(value), value);
            }
            else if (containerFlag == Instructions::MemorySpotAhead)
            {
                TypeMacros::u32 address = this->read32();
                this->HEAP[address] = static_cast<TypeMacros::u8>(value);
            }
        }
        else if (this->IR == Instructions::vadd)
        {
            this->ACC = this->IREGS[0] + this->IREGS[1];
        }
        else if (this->IR == Instructions::vsub)
        {
            this->ACC = this->IREGS[0] - this->IREGS[1];
        }
        else if (this->IR == Instructions::vdiv)
        {
            this->ACC = static_cast<TypeMacros::i32>(this->IREGS[0] / this->IREGS[1]);
        }
        else if (this->IR == Instructions::vmul)
        {
            this->ACC = this->IREGS[0] * this->IREGS[1];
        }
        else if (this->IR == Instructions::vmod)
        {
            this->ACC = this->IREGS[0] % this->IREGS[1];
        }
        else if (this->IR == Instructions::vand)
        {
            this->ACC = this->IREGS[0] & this->IREGS[1];
        }
        else if (this->IR == Instructions::vor)
        {
            this->ACC = this->IREGS[0] | this->IREGS[1];
        }
        else if (this->IR == Instructions::vnot)
        {
            this->ACC = ~this->IREGS[0];
        }
        else if (this->IR == Instructions::vlshift)
        {
            this->ACC = this->IREGS[0] << this->IREGS[1];
        }
        else if (this->IR == Instructions::vrshift)
        {
            this->ACC = this->IREGS[0] >> this->IREGS[1];
        }
        else if (this->IR == Instructions::ujmp)
        {
            TypeMacros::u8 valueFlag = this->PROGRAM[this->PC++];
            TypeMacros::i32 value;

            if (valueFlag == Instructions::ImmediateIntegerAhead)
            {
                value = this->read32();
            }
            else if (valueFlag == Instructions::MemorySpotAhead)
            {
                TypeMacros::u32 address = this->read32();
                value = this->HEAP[address];
            }
            else if (valueFlag == Instructions::RegisterAhead)
            {
                value = static_cast<TypeMacros::i32>(get_register(*this, this->PROGRAM[this->PC++]));
            }

            this->PC = value;
        }
        else if (this->IR == Instructions::syscall)
        {
            if (this->SR == Syscalls::StdoutWrite)
            {
                SystemIO::printchar(static_cast<TypeMacros::u8>(this->IREGS[0]));
                SystemIO::flush();
            }
        }
        else if (this->IR == Instructions::halt)
        {
            return VMErrors::HALTED;
        }
        else
        {
            SystemIO::printf("Instruction not defined, %u\n", this->PROGRAM[this->IR]);
            return VMErrors::INSTRUCTION_NOT_DEFINED;
        }
    }

    return VMErrors::PASS;
}

void VirtualMachine::register_dump()
{
    // Get the stdout stream
    EmbediFileStream* stream = allFiles.get_handle("stdout");
    if (stream == nullptr)
        return;

    // Prepare the buffers for printing
    // Print specific registers
    char text[256];
    int len;
    len = snprintf(text, 256, "PC: %u ACC: %d DATA: %d IR: %u SR: %d\n", this->PC, this->ACC, this->DAT, this->IR, this->SR);
    stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);

    // Print the integer registers
    for (int i = 0; i < INTEGER_REGISTER_SPACE; i++)
    {
        len = snprintf(text, 256, "R%d: %d\n", i, static_cast<int>(this->IREGS[i]));
        stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);
    }

    // Print the float registers
    for (int i = 0; i < FLOAT_REGISTER_SPACE; i++)
    {
        len = snprintf(text, 256, "F%d: %f\n", i, this->FREGS[i]);
        stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);
    }

    // Print stack register
    len = snprintf(text, 256, "SP: %d\n", this->SP);
    stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);
}