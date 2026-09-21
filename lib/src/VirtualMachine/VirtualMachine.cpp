#include "VirtualMachine.hpp"

#define __VM_PROCESS_VALUE__(vf, value)                                                                                                \
        const TypeMacros::u8 vf = this->read8();                                                                                       \
        TypeMacros::i32 value;                                                                                                         \
        do {                                                                                                                           \
            switch (vf) {                                                                                                              \
                case Instructions::ImmediateIntegerAhead:                                                                              \
                    value = this->read32();                                                                                            \
                    break;                                                                                                             \
                case Instructions::MemorySpotAhead: {                                                                                  \
                    const TypeMacros::u32 address = this->read32();                                                                    \
                    value =                                                                                                            \
                        (static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address])     << 24) |                                      \
                        (static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address + 1]) << 16) |                                      \
                        (static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address + 2]) << 8)  |                                      \
                        (static_cast<TypeMacros::i32>(this->PROGRAM_MEMORY[address + 3]));                                             \
                    break;                                                                                                             \
                }                                                                                                                      \
                case Instructions::RegisterAhead: {                                                                                    \
                    const TypeMacros::u8 r = this->read8();                                                                            \
                    value = static_cast<TypeMacros::i32>(get_integer_register(*this, r));                                              \
                    break;                                                                                                             \
                }                                                                                                                      \
                default:                                                                                                               \
                    SystemIO::fprintf(this->vmerr, "Value not supported by the container, PC = %u & value flag = %u\n", this->PC, vf); \
                    return VMErrors::VALUE_NOT_SUPPORTED;                                                                              \
            }                                                                                                                          \
        } while(false)

#define __VM_PROCESS_AND_SET_CONTAINER__(cf, value)                                                                                     \
        const TypeMacros::u8 cf = this->read8();                                                                                        \
        do {                                                                                                                            \
            switch (cf) {                                                                                                               \
                case Instructions::RegisterAhead: {                                                                                     \
                    const TypeMacros::u8 r = this->read8();                                                                             \
                    set_register(*this, r, value, value);                                                                               \
                    break;                                                                                                              \
                }                                                                                                                       \
                case Instructions::MemorySpotAhead: {                                                                                   \
                    const TypeMacros::u32 address = this->read32();                                                                     \
                    this->write32(address, value);                                                                                      \
                    break;                                                                                                              \
                }                                                                                                                       \
                default:                                                                                                                \
                    SystemIO::fprintf(this->vmerr, "Container not supported in the VM, PC = %u & container flag = %u\n", this->PC, cf); \
                    return VMErrors::CONTAINER_NOT_SUPPORTED;                                                                           \
            }                                                                                                                           \
        } while (false)

#define __VM_PROCESS_AND_INC_CONTAINER__(cf, delta)                                                                                     \
        const TypeMacros::u8 cf = this->read8();                                                                                        \
        do {                                                                                                                            \
            switch (cf) {                                                                                                               \
                case Instructions::RegisterAhead: {                                                                                     \
                    const TypeMacros::u8 r = this->read8();                                                                             \
                    delta_add_register(*this, r, delta, delta);                                                                         \
                    break;                                                                                                              \
                }                                                                                                                       \
                case Instructions::MemorySpotAhead: {                                                                                   \
                    const TypeMacros::u32 address = this->read32();                                                                     \
                    this->write32(address, this->get32(address) + delta);                                                               \
                    break;                                                                                                              \
                }                                                                                                                       \
                default:                                                                                                                \
                    SystemIO::fprintf(this->vmerr, "Container not supported in the VM, PC = %u & container flag = %u\n", this->PC, cf); \
                    return VMErrors::CONTAINER_NOT_SUPPORTED;                                                                           \
            }                                                                                                                           \
        } while (false)

#define __VM_PROCESS_AND_DEC_CONTAINER__(cf, delta)                                                                                     \
        const TypeMacros::u8 cf = this->read8();                                                                                        \
        do {                                                                                                                            \
            switch (cf) {                                                                                                               \
                case Instructions::RegisterAhead: {                                                                                     \
                    const TypeMacros::u8 r = this->read8();                                                                             \
                    delta_add_register(*this, r, -delta, -delta);                                                                       \
                    break;                                                                                                              \
                }                                                                                                                       \
                case Instructions::MemorySpotAhead: {                                                                                   \
                    const TypeMacros::u32 address = this->read32();                                                                     \
                    this->write32(address, this->get32(address) - delta);                                                               \
                    break;                                                                                                              \
                }                                                                                                                       \
                default:                                                                                                                \
                    SystemIO::fprintf(this->vmerr, "Container not supported in the VM, PC = %u & container flag = %u\n", this->PC, cf); \
                    return VMErrors::CONTAINER_NOT_SUPPORTED;                                                                           \
            }                                                                                                                           \
        } while (false)

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
            SystemIO::fprintf(vm.vmerr, "Operation set_register error : Unsafe mode is not enabled, rcode = %u\n", r);
            vm.ERROR_CODE = VMErrors::UNSAFE_MODE_NOT_ENABLED;
        }
    }
    else if (r == 251)
    {
        if (vm.UNSAFE_MODE) {
            vm.STACK_POINTER = reinterpret_cast<TypeMacros::u8*>(i);
        } else {
            SystemIO::fprintf(vm.vmerr, "Operation set_register error : Unsafe mode is not enabled, rcode = %u\n", r);
            vm.ERROR_CODE = VMErrors::UNSAFE_MODE_NOT_ENABLED;
        }
    }
    else
    {
        SystemIO::fprintf(vm.vmerr, "Register definition not found, rcode = %u\n", r);
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
                SystemIO::fprintf(vm.vmerr, "Stack overflow, SP went out of bounds %l\n", reinterpret_cast<TypeMacros::u64>(vm.STACK_POINTER));
                vm.ERROR_CODE = VMErrors::STACK_OVERFLOW;
                return;
            } else if (vm.STACK_UNDERFLOW_CHECK(di)) {
                SystemIO::fprintf(vm.vmerr, "Stack underflow, SP went out of bounds %l\n", reinterpret_cast<TypeMacros::u64>(vm.STACK_POINTER));
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
                SystemIO::fprintf(vm.vmerr, "Heap address went out of bounds %l\n", reinterpret_cast<TypeMacros::u64>(vm.HEAP_POINTER));
                vm.ERROR_CODE = VMErrors::INVALID_HEAP_ADDRESS;
                return;
            }
        }

        vm.HEAP_POINTER += di;
    }
    else
    {
        SystemIO::fprintf(vm.vmerr, "Register definition not found, rcode = %u\n", r);
        vm.ERROR_CODE = VMErrors::REGISTER_NOT_DEFINED;
    }
}

inline TypeMacros::i32 get_integer_register(VirtualMachineV2& vm, const TypeMacros::u8 r)
{
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
        SystemIO::fprintf(vm.vmerr, "Register definition not found, rcode = %u\n", r);
        vm.ERROR_CODE = VMErrors::REGISTER_NOT_DEFINED;
        return -1;
    }
}

bool VirtualMachineV2::prepare(
    EmbediFileStream* out,
    EmbediFileStream* in,
    EmbediFileStream* err,
    EmbediAllocator* allocator,
    TypeMacros::u8* program,
    TypeMacros::u32 progSize,
    TypeMacros::u32 startingHeapSize
)
{
    this->vmout = out;
    this->vmin  = in;
    this->vmerr = err;

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

    this->ERROR_CODE = VMErrors::PASS;

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
    TypeMacros::u8 new_line[] = {10};
    char text[256];
    int len;
    len = snprintf(text, 256, "PC: %u ACC: %d DATA: %d IR: %u SR: %d", this->PC, this->ACC, this->DAT, this->IR, this->SR);
    stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);

    stream->write(new_line, 1);

    // Print the integer registers
    for (TypeMacros::u8 i = 0; i < INTEGER_REGISTER_SPACE; i++)
    {
        len = snprintf(text, 256, "|R%d: %d|", i, static_cast<int>(this->IREGS[i]));
        stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);
    }

    stream->write(new_line, 1);

    // Print the float registers
    for (TypeMacros::u8 i = 0; i < FLOAT_REGISTER_SPACE; i++)
    {
        len = snprintf(text, 256, "|F%d: %f|", i, this->FREGS[i]);
        stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);
    }

    // Print stack register
    len = snprintf(text, 256, "\nSP: %p STACK POINTER OFFSET: \n", this->STACK_POINTER, static_cast<unsigned VMWord>(this->STACK_POINTER - this->STACK));
    stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);

    for (TypeMacros::u32 i = this->programSize; i < this->programSize + embedi_min(5UL, STACK_SIZE); i++)
    {
        len = snprintf(text, 256, "|STACK[%u]: %u|", i, this->STACK[i - this->programSize]);
        stream->write(reinterpret_cast<const TypeMacros::u8*>(text), len);
    }

    stream->write(reinterpret_cast<const TypeMacros::u8*>(".....\n"), 6);
}

int VirtualMachineV2::start_program()
{
#ifdef MAINTAIN_STABLE_CLOCK

    TypeMacros::u32 batchNumber = 0;
    TypeMacros::u32 batchTime = micros();
    const TypeMacros::u32 targetBatchTime = TIME_FOR_ONE_INSTRUCTION * BATCH_SIZE;

#endif

    // ! I am scared of implicit type conversions and overflows (T_T)
    // ! Anyways this is the first iteration of the operating system and
    // ! second iteration of virtual machine I will be just happy if it worked
    while (this->PC < this->programSize && this->PC < this->chunkSize)
    {
        if (this->ERROR_CODE != VMErrors::PASS && !this->UNSAFE_MODE)
            return this->ERROR_CODE;

        this->IR = this->read8();

        switch (this->IR)
        {
            case Instructions::noop:
                break;

            case Instructions::set: {
                __VM_PROCESS_VALUE__(vf, value);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, value);
            } break;

            case Instructions::vadd:
                this->ACC = this->IREGS[0] + this->IREGS[1];
                break;

            case Instructions::vsub:
                this->ACC = this->IREGS[0] - this->IREGS[1];
                break;

            case Instructions::vdiv:
                this->ACC = static_cast<TypeMacros::i32>(this->IREGS[0] / this->IREGS[1]);
                break;

            case Instructions::vmul:
                this->ACC = this->IREGS[0] * this->IREGS[1];
                break;

            case Instructions::vmod:
                this->ACC = this->IREGS[0] % this->IREGS[1];
                break;

            case Instructions::vand:
                this->ACC = this->IREGS[0] & this->IREGS[1];
                break;
            
            case Instructions::vor:
                this->ACC = this->IREGS[0] | this->IREGS[1];
                break;

            case Instructions::vnot:
                this->ACC = ~this->IREGS[0];
                break;

            case Instructions::vlshift:
                this->ACC = this->IREGS[0] << this->IREGS[1];
                break;

            case Instructions::vrshift:
                this->ACC = this->IREGS[0] >> this->IREGS[1];
                break;

            case Instructions::vxor:
                this->ACC = this->IREGS[0] ^ this->IREGS[1];
                break;

            case Instructions::ujmp: {
                __VM_PROCESS_VALUE__(vf, addr);
                this->PC = static_cast<TypeMacros::u32>(addr);
            } break;

            case Instructions::jmpi0: {
                __VM_PROCESS_VALUE__(vf, addr);                
                if (this->ACC == 0)
                    this->PC = static_cast<TypeMacros::u32>(addr);
            } break;

            case Instructions::jmpip: {
                __VM_PROCESS_VALUE__(vf, addr);            
                if (this->ACC > 0)
                    this->PC = static_cast<TypeMacros::u32>(addr);
            } break;

            case Instructions::jmpin: {
                __VM_PROCESS_VALUE__(vf, addr);
                if (this->ACC < 0)
                    this->PC = static_cast<TypeMacros::u32>(addr);
            } break;

            case Instructions::syscall: {
                switch (this->SR)
                {
                    case Syscalls::StdoutWrite: {
                        if (this->vmout == nullptr)
                        {
                            SystemIO::fprintf(this->vmerr, "Output Stream not defined\n");
                            return VMErrors::STREAM_NOT_DEFINED;
                        }

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
                    } break;

                    case Syscalls::PinModeSet:
                        pinMode(
                            static_cast<TypeMacros::u8>(this->IREGS[0]),
                            static_cast<TypeMacros::u8>(this->IREGS[1])
                        );
                        break;

                    case Syscalls::PinSet:
                        digitalWrite(
                            static_cast<TypeMacros::u8>(this->IREGS[0]),
                            static_cast<TypeMacros::u8>(this->IREGS[1])
                        );
                        break;

                    case Syscalls::PinRead: {
                        if (this->STACK_OVERFLOW_CHECK(1))
                            return VMErrors::STACK_OVERFLOW;

                        *(this->STACK_POINTER++) = digitalRead(static_cast<TypeMacros::u8>(this->IREGS[0]));
                    } break;

                    case Syscalls::CMDRun:
                        SystemIO::fprintf(this->vmerr, "Feature: Syscall - CMDRun not implemented\n");
                        return VMErrors::FEATURE_NOT_IMPLEMENTED;
                    
                    case Syscalls::StdinAvail: {
                        if (this->vmin == nullptr)
                        {
                            SystemIO::fprintf(this->vmerr, "Input Stream not defined\n");
                            return VMErrors::STREAM_NOT_DEFINED;
                        }

                        if (this->STACK_OVERFLOW_CHECK(4))
                            return VMErrors::STACK_OVERFLOW;

                        TypeMacros::i32 value = SystemIO::favailable(this->vmin);

                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((value >> 24) & 0xFF);
                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((value >> 16) & 0xFF);
                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((value >> 8) & 0xFF);
                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>(value & 0xFF);
                    } break;

                    case Syscalls::StdinRead: {
                        if (this->vmin == nullptr)
                        {
                            SystemIO::fprintf(this->vmerr, "Input Stream not defined\n");
                            return VMErrors::STREAM_NOT_DEFINED;
                        }

                        if (this->STACK_OVERFLOW_CHECK(1))
                            return VMErrors::STACK_OVERFLOW;

                        TypeMacros::u8 value = this->vmin->read();

                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>(value & 0xFF);                      
                    } break;

#ifdef COMPILE_WITH_DISPLAY_INITIALIZATION

                    case Syscalls::DisplayClear: {
                        clear_display();
                    } break;

                    case Syscalls::DisplaySetCursor: {
                        set_cursor_position(
                            static_cast<TypeMacros::u32>(this->IREGS[0]),
                            static_cast<TypeMacros::u32>(this->IREGS[1])
                        );
                    } break;

                    case Syscalls::DisplaySetTextColor: {
                        set_text_color(static_cast<TypeMacros::u16>(this->IREGS[0]));
                    } break;

                    case Syscalls::DisplaySetBackgroundColor: {
                        set_background_color(static_cast<TypeMacros::u16>(this->IREGS[0]));
                    } break;

                    case Syscalls::DisplaySetHighlightColor: {
                        set_highlight_color(static_cast<TypeMacros::u16>(this->IREGS[0]));
                    } break;

                    case Syscalls::DisplayGetCursor: {
                        if (this->STACK_OVERFLOW_CHECK(8))
                            return VMErrors::STACK_OVERFLOW;

                        DCPos pos = get_cursor_position();

                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((pos.x >> 24) & 0xFF);
                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((pos.x >> 16) & 0xFF);
                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((pos.x >> 8) & 0xFF);
                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>(pos.x & 0xFF);

                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((pos.y >> 24) & 0xFF);
                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((pos.y >> 16) & 0xFF);
                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((pos.y >> 8) & 0xFF);
                        *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>(pos.y & 0xFF);
                    } break;

#endif
                }
            } break;

            case Instructions::halt:
                return VMErrors::HALTED;

            case Instructions::push: {
                __VM_PROCESS_VALUE__(vf, value);

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
            } break;

            case Instructions::pop: {
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
            } break;

            case Instructions::vinc: {
                __VM_PROCESS_VALUE__(vf, value);
                __VM_PROCESS_AND_INC_CONTAINER__(cf, value);
            } break;

            case Instructions::vdec: {
                __VM_PROCESS_VALUE__(vf, value);
                __VM_PROCESS_AND_DEC_CONTAINER__(cf, value);
            } break;

            case Instructions::hextend: {
                __VM_PROCESS_VALUE__(vf, value);

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
            } break;

            case Instructions::write: {
                __VM_PROCESS_VALUE__(vf, value);

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
            } break;

            case Instructions::read: {
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
            } break;

            case Instructions::uwrite: {
                if (this->UNSAFE_MODE)
                {
                    __VM_PROCESS_VALUE__(vf, value);

                    VMRPtr address = static_cast<VMRPtr>(value);

                    const TypeMacros::u8 payload = this->read8();
                    *reinterpret_cast<TypeMacros::u8*>(address) = payload;
                }
                else
                {
                    SystemIO::fprintf(this->vmerr, "Operation uwrite requires unsafe mode to be enabled\n");
                    return VMErrors::UNSAFE_MODE_NOT_ENABLED;
                }
            } break;

            case Instructions::uread: {
                if (this->UNSAFE_MODE)
                {
                    __VM_PROCESS_VALUE__(vf, value);

                    VMRPtr address = static_cast<VMRPtr>(value);

                    this->ACC = static_cast<TypeMacros::i32>(*reinterpret_cast<TypeMacros::u8*>(address));
                }
                else
                {
                    SystemIO::fprintf(this->vmerr, "Operation uread requires unsafe mode to be enabled\n");
                    return VMErrors::UNSAFE_MODE_NOT_ENABLED;
                }
            } break;

            case Instructions::eum:
                this->UNSAFE_MODE = true;
                break;

            case Instructions::dum:
                this->UNSAFE_MODE = false;
                break;

            case Instructions::vaddi: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 + v2);
            } break;

            case Instructions::vsubi: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 - v2);
            } break;

            case Instructions::vmuli: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 * v2);
            } break;

            case Instructions::vdivi: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 / v2);
            } break;

            case Instructions::vmodi: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 % v2);
            } break;

            case Instructions::vandi: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 & v2);
            } break;

            case Instructions::vori: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 | v2);
            } break;

            case Instructions::vnoti: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, ~v1);
            } break;

            case Instructions::vxori: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 ^ v2);
            } break;

            case Instructions::vlshifti: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 << v2);
            } break;

            case Instructions::vrshifti: {
                __VM_PROCESS_VALUE__(vf1, v1);
                __VM_PROCESS_VALUE__(vf2, v2);
                __VM_PROCESS_AND_SET_CONTAINER__(cf, v1 >> v2);
            } break;

            case Instructions::call: {
                if (this->STACK_OVERFLOW_CHECK(4))
                    return VMErrors::STACK_OVERFLOW;

                *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((this->PC >> 24) & 0xFF);
                *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((this->PC >> 16) & 0xFF);
                *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>((this->PC >> 8) & 0xFF);
                *(this->STACK_POINTER++) = static_cast<TypeMacros::u8>(this->PC & 0xFF);

                __VM_PROCESS_VALUE__(vf, addr);

                this->PC = static_cast<TypeMacros::u32>(addr);
            } break;

            case Instructions::ret: {
                if (this->STACK_UNDERFLOW_CHECK(4))
                    return VMErrors::STACK_UNDERFLOW;
                
                this->PC = (
                    (static_cast<TypeMacros::u32>( *(--this->STACK_POINTER) ) & 0xFF) |
                    (static_cast<TypeMacros::u32>( *(--this->STACK_POINTER) ) << 8)   |
                    (static_cast<TypeMacros::u32>( *(--this->STACK_POINTER) ) << 16)  |
                    (static_cast<TypeMacros::u32>( *(--this->STACK_POINTER) ) << 24)
                ) + 5;
                // + 4 is to reach to -> <instruction> <32 - bit address> ... <next_instruction>
                //                                                            ^^^^^^^^^^^^^^^^^^
            } break;

            case Instructions::clr:
                this->ACC = 0;
                break;

            case Instructions::clri: {
                __VM_PROCESS_AND_SET_CONTAINER__(cf, 0);
            } break;

            case Instructions::nopx: {
                __VM_PROCESS_VALUE__(vf, value);

#ifdef MAINTAIN_STABLE_CLOCK

                TypeMacros::u32 _inner_batchNumber = 0;
                TypeMacros::u32 _inner_batchTime = micros();

#endif

                // Simulating an instruction fetch
                // to maintain consistency among instruction execution
                for (TypeMacros::i32 i = 0; i < value; i++)
                {
                    if (this->ERROR_CODE != VMErrors::PASS && !this->UNSAFE_MODE)
                        return this->ERROR_CODE;

#ifdef MAINTAIN_STABLE_CLOCK

                    _inner_batchNumber++;
                    if (_inner_batchNumber >= BATCH_SIZE)
                    {
                        _inner_batchNumber = 0;
                        _inner_batchTime += targetBatchTime;

                        int32_t drift = _inner_batchTime - micros();

                        if (drift > 0) {
                            VM_DELAY_US(drift);
                        }
                    }
#endif
                }
            } break;

            case Instructions::jmpis: {
                __VM_PROCESS_VALUE__(cf, addr);
                if (this->CMP)
                    this->PC = static_cast<TypeMacros::u32>(addr);
            } break;

            case Instructions::rstcmp:
                this->CMP = 0;
                break;

            case Instructions::inicmp: {
                __VM_PROCESS_VALUE__(vf1, v1);
                TypeMacros::u8 oper = this->read8();
                __VM_PROCESS_VALUE__(vf2, v2);

                switch (oper)
                {
                    case Conditions::EQUALS:
                        this->CMP = v1 == v2;
                        break;

                    case Conditions::GREATER:
                        this->CMP = v1 > v2;
                        break;

                    case Conditions::GREATER_EQUALS:
                        this->CMP = v1 >= v2;
                        break;

                    case Conditions::LOWER:
                        this->CMP = v1 < v2;
                        break;

                    case Conditions::LOWER_EQUALS:
                        this->CMP = v1 <= v2;
                        break;

                    case Conditions::NOT_EQUALS:
                        this->CMP = v1 != v2;
                        break;
                }

            } break;

            default:
                SystemIO::fprintf(this->vmerr, "Instruction not defined, opcode = %u\n", this->IR);
                return VMErrors::INSTRUCTION_NOT_DEFINED;
        }

#ifdef MAINTAIN_STABLE_CLOCK

        batchNumber++;
        if (batchNumber >= BATCH_SIZE)
        {
            batchNumber = 0;
            batchTime += targetBatchTime;
            int32_t drift = batchTime - micros();

            if (drift > 0) {
                VM_DELAY_US(drift);
            }
        }

#endif
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
    this->vmerr               = nullptr;
    this->STACK_START_POINTER = nullptr;
    this->HEAP_START_POINTER  = nullptr;
}

// Delete the macros
#undef __VM_PROCESS_VALUE__
#undef __VM_PROCESS_AND_SET_CONTAINER__
#undef __VM_PROCESS_AND_INC_CONTAINER__
#undef __VM_PROCESS_AND_DEC_CONTAINER__