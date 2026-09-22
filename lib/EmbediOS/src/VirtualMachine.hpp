#ifndef __EMBEDI_VIRTUAL_MACHINE_HPP__
#define __EMBEDI_VIRTUAL_MACHINE_HPP__

#include <Arduino.h>
#include "TypeMacros.hpp"
#include "Defines.hpp"

#include "IOStreams.hpp"
#include "Allocator.hpp"
#include "InternalFunctions.hpp"

// #include "InternalFunctions.hpp"

constexpr TypeMacros::u8 INTEGER_REGISTER_SPACE = 16;
constexpr TypeMacros::u8 FLOAT_REGISTER_SPACE   = 16;
constexpr TypeMacros::u8 INTEGER_OFFSET         = 1;
constexpr TypeMacros::u8 FLOAT_OFFSET           = 20;

// TODO: IMPLEMENT IMMEDIATE INSTRUCTIONS
namespace Instructions
{
    constexpr TypeMacros::u8 noop     = 0;  // no operation
    constexpr TypeMacros::u8 set      = 1;  // set instruction 
    constexpr TypeMacros::u8 ujmp     = 3;  // unconditional jump
    constexpr TypeMacros::u8 jmpi0    = 4;  // jump if acc = 0
    constexpr TypeMacros::u8 jmpip    = 5;  // jump if acc > 0
    constexpr TypeMacros::u8 jmpin    = 6;  // jump if acc < 0
    constexpr TypeMacros::u8 vadd     = 7;  // acc = r0 + r1
    constexpr TypeMacros::u8 vsub     = 8;  // acc = r0 - r1
    constexpr TypeMacros::u8 vmul     = 9;  // acc = r0 * r1
    constexpr TypeMacros::u8 vdiv     = 10; // acc = r0 / r1
    constexpr TypeMacros::u8 vmod     = 11; // acc = r0 % r1
    constexpr TypeMacros::u8 vrshift  = 12; // acc = r0 << r1
    constexpr TypeMacros::u8 vlshift  = 13; // acc = r0 >> r1
    constexpr TypeMacros::u8 vand     = 14; // acc = r0 & r1
    constexpr TypeMacros::u8 vor      = 15; // acc = r0 | r1
    constexpr TypeMacros::u8 vnot     = 16; // acc = ~r0
    constexpr TypeMacros::u8 push     = 17; // stack <- acc; sp += (some value)
    constexpr TypeMacros::u8 pop      = 18; // stack -> acc; sp -= (some value)
    constexpr TypeMacros::u8 syscall  = 19; // specific os syscall
    constexpr TypeMacros::u8 halt     = 20; // exit vm
    constexpr TypeMacros::u8 vxor     = 21; // acc = r0 ^ r1
    constexpr TypeMacros::u8 vinc     = 22; // (some container) += (some value)
    constexpr TypeMacros::u8 vdec     = 23; // (some container) -= (some value)
    constexpr TypeMacros::u8 hextend  = 24; // heap region += (some value)
    constexpr TypeMacros::u8 write    = 25; // heap <- (some value)
    constexpr TypeMacros::u8 read     = 26; // (some value at heap pointer) -> acc
    constexpr TypeMacros::u8 uwrite   = 27; // ram unsafe write (byte specific)
    constexpr TypeMacros::u8 uread    = 28; // ram unsafe read (byte specific)
    constexpr TypeMacros::u8 eum      = 29; // enable unsafe mode
    constexpr TypeMacros::u8 dum      = 30; // disable unsafe mode
    constexpr TypeMacros::u8 vaddi    = 31; // <container> = <value> + <value>
    constexpr TypeMacros::u8 vsubi    = 32; // <container> = <value> - <value>
    constexpr TypeMacros::u8 vmuli    = 33; // <container> = <value> * <value>
    constexpr TypeMacros::u8 vdivi    = 34; // <container> = <value> / <value>
    constexpr TypeMacros::u8 vmodi    = 35; // <container> = <value> % <value>
    constexpr TypeMacros::u8 vandi    = 36; // <container> = <value> & <value>
    constexpr TypeMacros::u8 vori     = 37; // <container> = <value> | <value>
    constexpr TypeMacros::u8 vnoti    = 38; // <container> = ~<value>
    constexpr TypeMacros::u8 vxori    = 39; // <container> = <value> ^ <value>
    constexpr TypeMacros::u8 vlshifti = 40; // <container> = <value> << <value>
    constexpr TypeMacros::u8 vrshifti = 41; // <container> = <value> >> <value>
    constexpr TypeMacros::u8 call     = 42; // push pc onto the stack; jump to the label
    constexpr TypeMacros::u8 ret      = 43; // pop pc into ACC; jump back to ACC + 1
    constexpr TypeMacros::u8 clr      = 44; // acc = 0
    constexpr TypeMacros::u8 clri     = 45; // <container> = 0
    constexpr TypeMacros::u8 nopx     = 46; // noop for <value> clock cycles
    constexpr TypeMacros::u8 jmpis    = 47; // jump if cmp flag is set
    constexpr TypeMacros::u8 rstcmp   = 48; // reset cmp flag
    constexpr TypeMacros::u8 inicmp   = 49; // compare 2 values

    constexpr TypeMacros::u8 RegisterAhead         = 0;
    constexpr TypeMacros::u8 MemorySpotAhead       = 1;
    constexpr TypeMacros::u8 ImmediateIntegerAhead = 2;
    constexpr TypeMacros::u8 ImmediateFloatAhead   = 3;
}

namespace Conditions
{
    constexpr TypeMacros::u8 EQUALS         = 1;
    constexpr TypeMacros::u8 GREATER        = 2;
    constexpr TypeMacros::u8 GREATER_EQUALS = 3;
    constexpr TypeMacros::u8 LOWER          = 4;
    constexpr TypeMacros::u8 LOWER_EQUALS   = 5;
    constexpr TypeMacros::u8 NOT_EQUALS     = 6;
};

namespace Syscalls
{
    constexpr TypeMacros::i32 StdoutWrite = 1;
    constexpr TypeMacros::i32 PinModeSet  = 2;
    constexpr TypeMacros::i32 PinSet      = 3;
    constexpr TypeMacros::i32 PinRead     = 4;
    constexpr TypeMacros::i32 CMDRun      = 5;
    constexpr TypeMacros::i32 StdinAvail  = 6;
    constexpr TypeMacros::i32 StdinRead   = 7;

#ifdef COMPILE_WITH_SD_INITIALIZATION
    constexpr TypeMacros::i32 FileOpen    = 7;
    constexpr TypeMacros::i32 FileRead    = 8;
    constexpr TypeMacros::i32 FileWrite   = 9;
    constexpr TypeMacros::i32 FileClose   = 10;
    constexpr TypeMacros::i32 FileSeek    = 11;
    constexpr TypeMacros::i32 FileTell    = 12;
#endif

#ifdef COMPILE_WITH_DISPLAY_INITIALIZATION
    constexpr TypeMacros::i32 DisplayClear              = 13;
    constexpr TypeMacros::i32 DisplaySetCursor          = 14;
    constexpr TypeMacros::i32 DisplaySetTextColor       = 15;
    constexpr TypeMacros::i32 DisplaySetBackgroundColor = 16;
    constexpr TypeMacros::i32 DisplaySetHighlightColor  = 17;
    constexpr TypeMacros::i32 DisplayGetCursor          = 18;
#endif
};

namespace VMErrors
{
    constexpr int PASS                    =  0;
    constexpr int HALTED                  =  1;
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
    constexpr int VM_INTERRUPTED          = -13;
    constexpr int VALUE_NOT_SUPPORTED     = -14;
    constexpr int CONTAINER_NOT_SUPPORTED = -15;
    constexpr int STREAM_NOT_DEFINED      = -16;
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

struct VirtualMachine
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
    VMWord IREGS[INTEGER_REGISTER_SPACE] = {0};

    // Float GPRs
    TypeMacros::f64 FREGS[FLOAT_REGISTER_SPACE] = {0};

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

    // error debuggers (is used to interrupt a program)
    // warning, if the vm is in unsafe mode, then interrupting a program is not possible
    volatile TypeMacros::u8 ERROR_CODE = VMErrors::PASS;

    // compare flag
    bool CMP = false;

    // extreme unsafe mode
    bool UNSAFE_MODE = false;

    // I/O Streams
    EmbediFileStream* vmout = nullptr;
    EmbediFileStream* vmin  = nullptr;
    EmbediFileStream* vmerr = nullptr;

    // Checks for Stack overflow by checking SP + delta is greater than STACK memory
    constexpr bool STACK_OVERFLOW_CHECK(TypeMacros::u32 delta)
    { return this->STACK_POINTER + delta >= programSize + STACK_SIZE + this->PROGRAM_MEMORY; }

    // Checks for Stack underflow by checking SP - delta is lower than STACK memory
    constexpr bool STACK_UNDERFLOW_CHECK(TypeMacros::u32 delta)
    { return this->STACK_POINTER - delta < programSize + this->PROGRAM_MEMORY; }

    // Checks whether HP + delta is greater than or lower than Program memory
    constexpr bool INVALID_HEAP_ADDRESS_CHECK(TypeMacros::u32 delta)
    { return this->HEAP_POINTER + delta > chunkSize + this->PROGRAM_MEMORY || this->HEAP_POINTER + delta < this->PROGRAM_MEMORY + STACK_SIZE; }

    // Read 1 byte from the program memory
    TypeMacros::u8 read8()
    { return static_cast<TypeMacros::u8>(this->PROGRAM_MEMORY[this->PC++]); }

    // Read 2 bytes from the program memory
    TypeMacros::u16 read16()
    { return static_cast<TypeMacros::u16>(this->read8()) << 8 | static_cast<TypeMacros::u16>(this->read8()); }

    // Read 4 bytes from the program memory
    TypeMacros::u32 read32()
    { return static_cast<TypeMacros::u32>(this->read16()) << 16 | static_cast<TypeMacros::u32>(this->read16()); }

    // Read 8 bytes from the program memory
    TypeMacros::u64 read64()
    { return static_cast<TypeMacros::u64>(this->read32()) << 32 | static_cast<TypeMacros::u64>(this->read32()); }

    // Change 1 byte in the program memory at a specific index
    void write8(TypeMacros::u32 index, TypeMacros::u8 data)
    { this->PROGRAM_MEMORY[index] = data; }

    // Change 2 bytes in the program memory at a specific index
    void write16(TypeMacros::u32 index, TypeMacros::u16 data)
    { write8(index, static_cast<TypeMacros::u8>((data >> 8) & 0xFF)); write8(index + 1, static_cast<TypeMacros::u8>(data & 0xFF)); }

    // Change 4 bytes in the program memory at a specific index
    void write32(TypeMacros::u32 index, TypeMacros::u32 data)
    { write16(index, static_cast<TypeMacros::u16>((data >> 16) & 0xFFFF)); write16(index + 2, static_cast<TypeMacros::u16>(data & 0xFFFF)); }

    // Change 8 bytes in the program memory at a specific index
    void write64(TypeMacros::u32 index, TypeMacros::u64 data)
    { write32(index, static_cast<TypeMacros::u32>((data >> 32) & 0xFFFFFFFF)); write32(index + 4, static_cast<TypeMacros::u32>(data & 0xFFFFFFFF)); }

    // Get 1 byte in the program memory from a specific index
    TypeMacros::u8 get8(TypeMacros::u32 index)
    { return this->PROGRAM_MEMORY[index]; }

    // Get 2 bytes in the program memory from a specific index
    TypeMacros::u16 get16(TypeMacros::u32 index)
    { return (static_cast<TypeMacros::u16>(this->get8(index)) << 8) | static_cast<TypeMacros::u16>(this->get8(index + 1)); }

    // Get 4 bytes in the program memory from a specific index
    TypeMacros::u32 get32(TypeMacros::u32 index)
    { return (static_cast<TypeMacros::u32>(this->get16(index)) << 16) | static_cast<TypeMacros::u32>(this->get16(index + 2)); }

    // Get 8 bytes in the program memory from a specific index
    TypeMacros::u64 get64(TypeMacros::u32 index)
    { return (static_cast<TypeMacros::u64>(this->get32(index)) << 32) | static_cast<TypeMacros::u64>(this->get32(index + 4)); }

    // This function is a must for running a program
    // It prepares the buffers, allocates the PROGRAM_MEMORY on the heap
    // Resets the state of the Virtual Machine V2
    // and sets up the Stack and Heap pointers
    bool prepare(
        EmbediFileStream* out,
        EmbediFileStream* in,
        EmbediFileStream* err,
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