#include "VirtualMachine.hpp"

inline void set_register(VirtualMachineV2& vm, const TypeMacros::u8 r, const TypeMacros::i32 i, const TypeMacros::f64 f)
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
    else if (r == 252)
    {
        if (vm.UNSAFE_MODE) {
            vm.STACK_POINTER = reinterpret_cast<TypeMacros::u8*>(i);
        } else {
            vm.ERROR_CODE = VMErrors::UNSAFE_MODE_NOT_ENABLED;
        }
    }
    else if (r == 251)
    {
        if (vm.UNSAFE_MODE) {
            vm.STACK_POINTER = reinterpret_cast<TypeMacros::u8*>(i);
        } else {
            vm.ERROR_CODE = VMErrors::UNSAFE_MODE_NOT_ENABLED;
        }
    }
    else
    {
        SystemIO::printf("Register definition not found\n");
        vm.ERROR_CODE = VMErrors::REGISTER_NOT_DEFINED;
    }
}

inline void delta_add_register(VirtualMachineV2& vm, const TypeMacros::u8 r, const TypeMacros::i32 di, const TypeMacros::f64 df)
{
    if (r >= INTEGER_OFFSET && r <= (INTEGER_OFFSET + INTEGER_REGISTER_SPACE))
    {
        vm.IREGS[r - INTEGER_OFFSET] += di;
        return;
    }

    if (r >= FLOAT_OFFSET && r <= (FLOAT_OFFSET + FLOAT_REGISTER_SPACE))
    {
        vm.FREGS[r - FLOAT_OFFSET] += df;
        return;
    }

    if (r == 255)
    {
        vm.ACC += di;
    }
    else if (r == 254)
    {
        vm.DAT += di;
    }
    else if (r == 253)
    {
        vm.SR += di;
    }
    else if (r == 252)
    {
        if (!vm.UNSAFE_MODE) {
            if (di > 0 && vm.STACK_OVERFLOW_CHECK(di)) {
                vm.ERROR_CODE = VMErrors::STACK_OVERFLOW;
                return;
            } else if (vm.STACK_UNDERFLOW_CHECK(di)) {
                vm.ERROR_CODE = VMErrors::STACK_UNDERFLOW;
                return;
            }
        }

        vm.STACK_POINTER += di;
    }
    else if (r == 251)
    {
        if (!vm.UNSAFE_MODE) {
            if (vm.INVALID_HEAP_ADDRESS_CHECK(di)) {
                vm.ERROR_CODE = VMErrors::INVALID_HEAP_ADDRESS;
                return;
            }
        }

        vm.HEAP_POINTER += di;
    }
    else
    {
        SystemIO::printf("Register definition not found\n");
        vm.ERROR_CODE = VMErrors::REGISTER_NOT_DEFINED;
    }
}

inline TypeMacros::i32 get_integer_register(VirtualMachineV2& vm, const TypeMacros::u8 r) {
    if (r >= INTEGER_OFFSET && r <= (INTEGER_OFFSET + INTEGER_REGISTER_SPACE))
        return vm.IREGS[r - INTEGER_OFFSET];

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
    else if (r == 252)
    {
        return reinterpret_cast<VMWord>(vm.STACK_POINTER);
    }
    else if (r == 251)
    {
        return reinterpret_cast<VMWord>(vm.HEAP_POINTER);
    }
    else
    {
        SystemIO::perror("Register definition not found\n");
        vm.ERROR_CODE = VMErrors::REGISTER_NOT_DEFINED;
        return -1;
    }
}

bool VirtualMachineV2::prepare(
    EmbediFileStream* out,
    EmbediFileStream* in,
    EmbediAllocator* allocator,
    TypeMacros::u8* program,
    TypeMacros::u32 progSize,
    TypeMacros::u32 startingHeapSize
)
{
    this->vmout = out;
    this->vmin = in;

    TypeMacros::memptr space = allocator->alloc(progSize + STACK_SIZE + startingHeapSize);
    if (space == nullptr)
        return false;

    this->PROGRAM_MEMORY = static_cast<TypeMacros::u8*>(space);
    this->programSize = progSize;
    this->STACK = this->PROGRAM_MEMORY + progSize;
    this->STACK_POINTER = this->STACK;

    this->STACK_START_POINTER = this->STACK;

    if (startingHeapSize > 0) {
        this->HEAP_POINTER = this->STACK + STACK_SIZE;
        this->HEAP_START_POINTER = this->STACK + STACK_SIZE;
    } else this->HEAP_POINTER = nullptr;

    this->PC  = 0;
    this->ACC = 0;
    this->DAT = 0;
    this->IR  = 0;
    this->SR  = 0;

    for (TypeMacros::u8 i = 0; i < INTEGER_REGISTER_SPACE; i++)
        this->IREGS[i] = 0;

    for (TypeMacros::u8 i = 0; i < FLOAT_REGISTER_SPACE; i++)
        this->FREGS[i] = 0;

    memcpy(this->PROGRAM_MEMORY, program, progSize * sizeof(TypeMacros::u8));
    memset(this->PROGRAM_MEMORY + progSize, 0x00, STACK_SIZE + startingHeapSize);

    this->vmallocator = allocator;

    this->chunkSize = progSize + STACK_SIZE + startingHeapSize;

    return true;
}

void VirtualMachineV2::register_dump()
{
    // Get the stdout stream
    EmbediFileStream* stream = allFiles.get_handle(SYSOUT_NAME);
    if (stream == nullptr)
        return;

    // Prepare the buffers for printing
    // Print specific registers
    char text[256];
    int len;
    len = snprintf(text, 256, "PC: %u ACC: %d DATA: %d IR: %u SR: %d\n", this->PC, this->ACC, this->DAT, this->IR, this->SR);
    stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);

    // Print the integer registers
    for (TypeMacros::u8 i = 0; i < INTEGER_REGISTER_SPACE; i++)
    {
        len = snprintf(text, 256, "R%d: %d\n", i, static_cast<int>(this->IREGS[i]));
        stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);
    }

    // Print the float registers
    for (TypeMacros::u8 i = 0; i < FLOAT_REGISTER_SPACE; i++)
    {
        len = snprintf(text, 256, "F%d: %f\n", i, this->FREGS[i]);
        stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);
    }

    // Print stack register
    len = snprintf(text, 256, "SP: %p STACK POINTER OFFSET: \n", this->STACK_POINTER, static_cast<unsigned VMWord>(this->STACK_POINTER - this->STACK));
    stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);

    for (TypeMacros::u32 i = this->programSize; i < this->programSize + __min(64UL, STACK_SIZE); i++)
    {
        len = snprintf(text, 256, "STACK[%u]: %u\n", i, this->STACK[i - this->programSize]);
        stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);
    }
}

int VirtualMachineV2::start_program()
{
    // ! I am scared of implicit type conversions and overflows (T_T)
    while (this->PC < this->programSize && this->PC < this->chunkSize)
    {
        if (this->ERROR_CODE != VMErrors::PASS && !UNSAFE_MODE)
            return this->ERROR_CODE;
        
        this->IR = this->read8();

        if (this->IR == Instructions::noop)
        {
            continue;
        }
        else if (this->IR == Instructions::set)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::i32 value;

            if (vf == Instructions::ImmediateIntegerAhead) {
                value = this->read32();
            } else if (vf == Instructions::MemorySpotAhead) {
                const TypeMacros::u32 address = this->read32();
                value = static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address]);
            } else if (vf == Instructions::RegisterAhead) {
                const TypeMacros::u8 r = this->read8();
                value = static_cast<TypeMacros::i32>(get_integer_register(*this, r));
            }

            const TypeMacros::u8 cf = this->read8();
            if (cf == Instructions::RegisterAhead) {
                const TypeMacros::u8 r = this->read8();
                set_register(*this, r, value, value);
            } else if (cf == Instructions::MemorySpotAhead) {
                const TypeMacros::u32 address = this->read32();
                this->PROGRAM_MEMORY[address] = static_cast<TypeMacros::u8>(value);
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
        else if (this->IR == Instructions::vxor)
        {
            this->ACC = this->IREGS[0] ^ this->IREGS[1];
        }
        else if (this->IR == Instructions::ujmp)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::u32 addr;

            if (vf == Instructions::ImmediateIntegerAhead)
            {
                addr = this->read32();
            }
            else if (vf == Instructions::RegisterAhead)
            {
                const TypeMacros::u8 r = this->read8();
                addr = static_cast<TypeMacros::u32>(get_integer_register(*this, r));
            }
            else if (vf == Instructions::MemorySpotAhead)
            {
                const TypeMacros::u32 address = this->read32();
                if (address >= this->chunkSize)
                    return VMErrors::INVALID_MEMORY_ADDRESS;

                this->PC = address;
                addr = this->read32();
            }

            this->PC = addr;
        }
        else if (this->IR == Instructions::jmpi0)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::u32 addr;

            if (vf == Instructions::ImmediateIntegerAhead)
            {
                addr = this->read32();
            }
            else if (vf == Instructions::RegisterAhead)
            {
                const TypeMacros::u8 r = this->read8();
                addr = static_cast<TypeMacros::u32>(get_integer_register(*this, r));
            }
            else if (vf == Instructions::MemorySpotAhead)
            {
                const TypeMacros::u32 address = this->read32();
                if (address >= this->chunkSize)
                    return VMErrors::INVALID_MEMORY_ADDRESS;

                const TypeMacros::u32 temp = this->PC;
                this->PC = address;
                addr = this->read32();
                this->PC = temp;
            }

            if (this->ACC == 0)
                this->PC = addr;
        }
        else if (this->IR == Instructions::jmpip)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::u32 addr;

            if (vf == Instructions::ImmediateIntegerAhead)
            {
                addr = this->read32();
            }
            else if (vf == Instructions::RegisterAhead)
            {
                const TypeMacros::u8 r = this->read8();
                addr = static_cast<TypeMacros::u32>(get_integer_register(*this, r));
            }
            else if (vf == Instructions::MemorySpotAhead)
            {
                const TypeMacros::u32 address = this->read32();
                if (address >= this->chunkSize)
                    return VMErrors::INVALID_MEMORY_ADDRESS;

                const TypeMacros::u32 temp = this->PC;
                this->PC = address;
                addr = this->read32();
                this->PC = temp;
            }

            if (this->ACC > 0)
                this->PC = addr;
        }
        else if (this->IR == Instructions::jmpin)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::u32 addr;

            if (vf == Instructions::ImmediateIntegerAhead)
            {
                addr = this->read32();
            }
            else if (vf == Instructions::RegisterAhead)
            {
                const TypeMacros::u8 r = this->read8();
                addr = static_cast<TypeMacros::u32>(get_integer_register(*this, r));
            }
            else if (vf == Instructions::MemorySpotAhead)
            {
                const TypeMacros::u32 address = this->read32();
                if (address >= this->chunkSize)
                    return VMErrors::INVALID_MEMORY_ADDRESS;

                const TypeMacros::u32 temp = this->PC;
                this->PC = address;
                addr = this->read32();
                this->PC = temp;
            }

            if (this->ACC < 0)
                this->PC = addr;
        }
        else if (this->IR == Instructions::syscall)
        {
            if (this->SR == Syscalls::StdoutWrite && this->vmout != nullptr)
            {
                TypeMacros::i32 writeFlag = this->IREGS[0];
                switch (writeFlag)
                {
                case 0:
                    SystemIO::fputc(this->vmout, static_cast<TypeMacros::u8>(this->IREGS[1]));
                    break;
                case 1:
                    SystemIO::fputs(this->vmout, reinterpret_cast<const char*>(&this->PROGRAM_MEMORY[this->IREGS[1]]));
                    break;
                }
                this->vmout->flush();
            }
            else if (this->SR == Syscalls::PinModeSet)
            {
                pinMode(this->IREGS[0], this->IREGS[1]);
            }
            else if (this->SR == Syscalls::PinSet)
            {
                digitalWrite(this->IREGS[0], this->IREGS[1]);
            }
            else if (this->SR == Syscalls::PinRead)
            {
                this->ACC = static_cast<TypeMacros::i32>(digitalRead(this->IREGS[0]));
            }
            else if (this->SR == Syscalls::CMDRun)
            {
                return VMErrors::FEATURE_NOT_IMPLEMENTED;
            }
        }
        else if (this->IR == Instructions::push)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::i32 value;

            if (vf == Instructions::ImmediateIntegerAhead) {
                value = this->read32();
            } else if (vf == Instructions::MemorySpotAhead) {
                const TypeMacros::u32 address = this->read32();
                value = static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address]);
            } else if (vf == Instructions::RegisterAhead) {
                const TypeMacros::u8 r = this->read8();
                value = static_cast<TypeMacros::i32>(get_integer_register(*this, r));
            }

            const TypeMacros::u8 offset = this->read8();

            if (this->STACK_OVERFLOW_CHECK(offset))
                return VMErrors::STACK_OVERFLOW;


            switch (offset)
            {
                case 1:
                    *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>(value & 0xFF);
                    break;

                case 2:
                    *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((value >> 8) & 0xFF);
                    *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>(value & 0xFF);
                    break;

                case 4:
                    *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((value >> 24) & 0xFF);
                    *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((value >> 16) & 0xFF);
                    *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((value >> 8) & 0xFF);
                    *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>(value & 0xFF);
                    break;

                default:
                    return VMErrors::INVALID_STACK_OFFSET;
            }
        }
        else if (this->IR == Instructions::pop)
        {
            const TypeMacros::u8 offset = this->read8();

            if (this->STACK_UNDERFLOW_CHECK(offset))
                return VMErrors::STACK_OVERFLOW;

            switch (offset)
            {
                case 1:
                    this->ACC = static_cast<TypeMacros::i32>( *(--this->STACK_POINTER) );
                    break;

                case 2:
                    this->ACC = (static_cast<TypeMacros::i32>( *(--this->STACK_POINTER) ) & 0xFF) |
                                (static_cast<TypeMacros::i32>( *(--this->STACK_POINTER) ) << 8);
                    break;

                case 4:
                    this->ACC = (static_cast<TypeMacros::i32>( *(--this->STACK_POINTER) ) & 0xFF) |
                                (static_cast<TypeMacros::i32>( *(--this->STACK_POINTER) ) << 8)   |
                                (static_cast<TypeMacros::i32>( *(--this->STACK_POINTER) ) << 16)  |
                                (static_cast<TypeMacros::i32>( *(--this->STACK_POINTER) ) << 24);
                    break;

                default:
                    return VMErrors::INVALID_STACK_OFFSET;
            }
        }
        else if (this->IR == Instructions::vinc)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::i32 value;

            if (vf == Instructions::ImmediateIntegerAhead) {
                value = this->read32();
            } else if (vf == Instructions::MemorySpotAhead) {
                const TypeMacros::u32 address = this->read32();
                value = static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address]);
            } else if (vf == Instructions::RegisterAhead) {
                const TypeMacros::u8 r = this->read8();
                value = static_cast<TypeMacros::i32>(get_integer_register(*this, r));
            }

            const TypeMacros::u8 cf = this->read8();
            if (cf == Instructions::RegisterAhead) {
                const TypeMacros::u8 r = this->read8();
                delta_add_register(*this, r, value, value);
            } else if (cf == Instructions::MemorySpotAhead) {
                const TypeMacros::u32 address = this->read32();
                this->PROGRAM_MEMORY[address] += static_cast<TypeMacros::u8>(value);
            }
        }
        else if (this->IR == Instructions::vdec)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::i32 value;

            if (vf == Instructions::ImmediateIntegerAhead) {
                value = this->read32();
            } else if (vf == Instructions::MemorySpotAhead) {
                const TypeMacros::u32 address = this->read32();
                value = static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address]);
            } else if (vf == Instructions::RegisterAhead) {
                const TypeMacros::u8 r = this->read8();
                value = static_cast<TypeMacros::i32>(get_integer_register(*this, r));
            }

            const TypeMacros::u8 cf = this->read8();
            if (cf == Instructions::RegisterAhead) {
                const TypeMacros::u8 r = this->read8();
                delta_add_register(*this, r, -value, -value);
            } else if (cf == Instructions::MemorySpotAhead) {
                const TypeMacros::u32 address = this->read32();
                this->PROGRAM_MEMORY[address] -= static_cast<TypeMacros::u8>(value);
            }
        }
        else if (this->IR == Instructions::hextend)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::i32 value;

            if (vf == Instructions::ImmediateIntegerAhead) {
                value = this->read32();
            } else if (vf == Instructions::MemorySpotAhead) {
                const TypeMacros::u32 address = this->read32();
                value = static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address]);
            } else if (vf == Instructions::RegisterAhead) {
                const TypeMacros::u8 r = this->read8();
                value = static_cast<TypeMacros::i32>(get_integer_register(*this, r));
            }

            TypeMacros::u8* buffer = static_cast<TypeMacros::u8*>(
                this->vmallocator->reshape(
                    this->PROGRAM_MEMORY,
                    static_cast<TypeMacros::u32>((static_cast<TypeMacros::i32>(this->chunkSize) + value) * sizeof(TypeMacros::u8))
                )
            ); if (buffer == nullptr)
                return VMErrors::ALLOCATION_FAILED;

            this->PROGRAM_MEMORY = buffer;
            this->chunkSize = static_cast<TypeMacros::u32>(static_cast<TypeMacros::i32>(this->chunkSize) + value);
            
            if (this->HEAP_POINTER == nullptr)
            {
                this->HEAP_POINTER = this->STACK + STACK_SIZE;
                this->HEAP_START_POINTER = this->STACK + STACK_SIZE;
            }
        }
        else if (this->IR == Instructions::write)
        {
            const TypeMacros::u8 vf = this->read8();
            TypeMacros::i32 value;

            if (vf == Instructions::ImmediateIntegerAhead) {
                value = this->read32();
            } else if (vf == Instructions::MemorySpotAhead) {
                const TypeMacros::u32 address = this->read32();
                value = static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address]);
            } else if (vf == Instructions::RegisterAhead) {
                const TypeMacros::u8 r = this->read8();
                value = static_cast<TypeMacros::i32>(get_integer_register(*this, r));
            }

            const TypeMacros::u8 offset = this->read8();

            if (this->INVALID_HEAP_ADDRESS_CHECK(offset))
                return VMErrors::INVALID_HEAP_ADDRESS;


            switch (offset)
            {
                case 1:
                    *(this->HEAP_POINTER) = static_cast<TypeMacros::u8>(value & 0xFF);
                    break;

                case 2:
                    *(this->HEAP_POINTER) =     static_cast<TypeMacros::u8>((value >> 8) & 0xFF);
                    *(this->HEAP_POINTER + 1) = static_cast<TypeMacros::u8>(value & 0xFF);
                    break;

                case 4:
                    *(this->HEAP_POINTER) =     static_cast<TypeMacros::u8>((value >> 24) & 0xFF);
                    *(this->HEAP_POINTER + 1) = static_cast<TypeMacros::u8>((value >> 16) & 0xFF);
                    *(this->HEAP_POINTER + 2) = static_cast<TypeMacros::u8>((value >> 8) & 0xFF);
                    *(this->HEAP_POINTER + 3) = static_cast<TypeMacros::u8>(value & 0xFF);
                    break;

                default:
                    return VMErrors::INVALID_HEAP_OFFSET;
            }
        }
        else if (this->IR == Instructions::read)
        {
            const TypeMacros::u8 offset = this->read8();

            if (this->INVALID_HEAP_ADDRESS_CHECK(offset))
                return VMErrors::INVALID_HEAP_ADDRESS;

            switch (offset)
            {
                case 1:
                    this->ACC = (static_cast<TypeMacros::i32>(*(this->HEAP_POINTER)) & 0xFF);
                    break;
                
                case 2:
                    this->ACC = (static_cast<TypeMacros::i32>(*(this->HEAP_POINTER)) & 0xFF)    |
                                (static_cast<TypeMacros::i32>(*(this->HEAP_POINTER + 1)) << 8);
                    break;

                case 4:
                    this->ACC = (static_cast<TypeMacros::i32>(*(this->HEAP_POINTER)) & 0xFF)    |
                                (static_cast<TypeMacros::i32>(*(this->HEAP_POINTER + 1)) << 8)  |
                                (static_cast<TypeMacros::i32>(*(this->HEAP_POINTER + 2)) << 16) |
                                (static_cast<TypeMacros::i32>(*(this->HEAP_POINTER + 3)) << 24);
                    break;
                
                default:
                    return VMErrors::INVALID_HEAP_OFFSET;
            }
        }
        else if (this->IR == Instructions::uwrite)
        {
            if (this->UNSAFE_MODE)
            {
                const TypeMacros::u8 vf = this->read8();
                VMRPtr value;

                if (vf == Instructions::ImmediateIntegerAhead) {
                    value = this->read32();
                } else if (vf == Instructions::MemorySpotAhead) {
                    const TypeMacros::u32 address = this->read32();
                    value = static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address]);
                } else if (vf == Instructions::RegisterAhead) {
                    const TypeMacros::u8 r = this->read8();
                    value = static_cast<TypeMacros::i32>(get_integer_register(*this, r));
                }

                const TypeMacros::u8 payload = this->read8();
                *reinterpret_cast<TypeMacros::u8*>(value) = payload;
            }
            else
                return VMErrors::UNSAFE_MODE_NOT_ENABLED;
        }
        else if (this->IR == Instructions::uread)
        {
        }
        else if (this->IR == Instructions::halt)
        {
            return VMErrors::HALTED;
        }
        else {
            SystemIO::fprintf(SYSERR_NAME, "Instruction not defined, opcode = %u", this->IR);
            return VMErrors::INSTRUCTION_NOT_DEFINED;
        }
    }

    return VMErrors::PASS;
}

int VirtualMachineV2::run()
{
    int rs = this->start_program();
    this->cleanup();
    return rs;
}

void VirtualMachineV2::cleanup()
{
    if (this->PROGRAM_MEMORY != nullptr)
    {
        assert(this->vmallocator != nullptr && "vmallocator is null");
        this->vmallocator->destroy(this->PROGRAM_MEMORY);
    }

    this->PROGRAM_MEMORY      = nullptr;
    this->STACK               = nullptr;
    this->STACK_POINTER       = nullptr;
    this->vmallocator         = nullptr;
    this->vmout               = nullptr;
    this->vmin                = nullptr;
    this->STACK_START_POINTER = nullptr;
    this->HEAP_START_POINTER  = nullptr;
}
