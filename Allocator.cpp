#include "Allocator.hpp"

// ------------------------------ BASIC ALLOCATOR ------------------------------
TypeMacros::memptr EmbediBasicAllocator::alloc(TypeMacros::u32 size)
{
    return malloc(size);
}

bool EmbediBasicAllocator::destroy(TypeMacros::memptr mem)
{
    if (mem != nullptr)
    {
        free(mem);
        return true;
    }
    else return false;
}

TypeMacros::memptr EmbediBasicAllocator::reshape(TypeMacros::memptr mem, TypeMacros::u32 size)
{
    TypeMacros::memptr space = realloc(mem, size);
    if (space != nullptr)
    {
        return space;
    }
    else return nullptr;
}

void EmbediBasicAllocator::dump(const char* streamName)
{
    SystemIO::fprintf(streamName, "Basic Allocator have no proper dump() method");
}

EmbediBasicAllocator basicAllocator;

// ------------------------------ ARENA ALLOCATOR ------------------------------
EmbediArenaAllocator::EmbediArenaAllocator(TypeMacros::u32 _blockSize) : blockSize(_blockSize)
{
    assert(_blockSize > 0 && "[EmbediArenaAllocator::EmbediArenaAllocator(TypeMacros::u32)] FATAL ERROR: BLOCK SIZE == 0, NOT SUPPORTED");
}

EmbediArenaAllocator::~EmbediArenaAllocator()
{
    if (this->blocks != nullptr)
    {
        for (int i = 0; i < this->numBlocks; i++)
            basicAllocator.destroy(this->blocks[i]);
    }

    basicAllocator.destroy(this->blocks);
}

bool EmbediArenaAllocator::arena_alloc_block()
{
    if (this->blocks == nullptr)
    {
        TypeMacros::memptr blockSpace = basicAllocator.alloc(sizeof(TypeMacros::memptr));

        if (blockSpace == nullptr)
            return false;

        this->blocks = static_cast<TypeMacros::memptr*>(blockSpace);
    }

    TypeMacros::memptr space = basicAllocator.reshape(this->blocks, ++this->numBlocks * sizeof(TypeMacros::memptr));
    if (space == nullptr)
        return false;

    TypeMacros::memptr blockSpace = basicAllocator.reshape(this->blocks, ++this->numBlocks * sizeof(TypeMacros::memptr));
    if (blockSpace == nullptr)
        return false;

    this->blocks = static_cast<TypeMacros::memptr*>(blockSpace);
    this->blocks[this->numBlocks - 1] = space;
    this->allocPointer = 0;

    return true;
}

TypeMacros::memptr EmbediArenaAllocator::alloc(TypeMacros::u32 size)
{
    if (this->numBlocks == 0)
    {
        if (!this->arena_alloc_block())
        {
            SystemIO::perror("[EmbediArenaAllocator::alloc(TypeMacros::u32)] ERROR: FAILED TO ALLOCATE MEMORY FOR NEW BLOCK");
            return nullptr;
        }
    }

    if (size > this->blockSize)
    {
        return nullptr;
    }

    if ((this->allocPointer + size) > this->blockSize)
    {
        if (!this->arena_alloc_block())
        {
            SystemIO::perror("[EmbediArenaAllocator::alloc(TypeMacros::u32)] ERROR: FAILED TO ALLOCATE MEMORY FOR NEW BLOCK");
            return nullptr;
        }
    }

    TypeMacros::memptr res = static_cast<u8*>(this->blocks[this->numBlocks - 1]) + this->allocPointer;
    this->allocPointer += size;

    return res;
}

bool EmbediArenaAllocator::destroy(TypeMacros::memptr mem)
{
    SystemIO::perror("[EmbediArenaAllocator::destroy(TypeMacros::memptr)] ERROR: destroy() method not implemented");
    return false;
}

TypeMacros::memptr EmbediArenaAllocator::reshape(TypeMacros::memptr mem, TypeMacros::u32 size)
{
    SystemIO::perror("[EmbediArenaAllocator::destroy(TypeMacros::memptr)] ERROR: reshape() method not implemented");
    return nullptr;
}

bool EmbediArenaAllocator::free_block(bool pseudo)
{
    if (this->blocks == nullptr || this->numBlocks == 0) return true;

    if (pseudo)
    {
        this->allocPointer = 0;
        return true;
    }
    else
    {
        basicAllocator.destroy(this->blocks[this->numBlocks - 1]);
        this->numBlocks--;

        if (this->numBlocks > 0)
        {
            TypeMacros::memptr* _ptemp = static_cast<TypeMacros::memptr*>(basicAllocator.reshape(this->blocks, this->numBlocks * sizeof(TypeMacros::memptr)));
            if (_ptemp == nullptr)
                return false;

            this->blocks = _ptemp;
            this->allocPointer = this->blockSize;
        }
        else
        {
            basicAllocator.destroy(this->blocks);
            this->blocks = nullptr;
            this->allocPointer = 0;
        }
    }

    return true;
}

void EmbediArenaAllocator::arena_free()
{
    if (this->blocks != nullptr)
    {
        for (TypeMacros::u32 i = 0; i < this->numBlocks; i++)
            basicAllocator.destroy(this->blocks[i]);

        basicAllocator.destroy(this->blocks);
        this->blocks = nullptr;
        this->numBlocks = 0;
    }
}

void EmbediArenaAllocator::dump(const char* streamName)
{
    SystemIO::fprintf(streamName, "EmbediArenaAllocator{\n\tblock size = %u\ntotal blocks = %u\n\n", this->blockSize, this->numBlocks);
    if (this->blocks != nullptr)
    {
        SystemIO::fprintf(streamName, "blocks:\n");
        for (TypeMacros::u32 i = 0; i < this->numBlocks; i++)
            SystemIO::fprintf(streamName, "\tblock{%u}: %p\n", i, this->blocks[i]);
    }
    SystemIO::fprintf(streamName, "}\n");
}

// ------------------------------ LINEAR ALLOCATOR ------------------------------
EmbediLinearAllocator::EmbediLinearAllocator(u8* buf, TypeMacros::u32 cap_size) : cap(cap_size)
{
    if (this->buffer == nullptr)
    {
        SystemIO::perror("[EmbediLinearAllocator::EmbediLinearAllocator(u8*, TypeMacros::u32)] ERROR: NULLPTR PASSED, CREATING BUFFER ON BASIC ALLOCATOR");

        this->buffer = static_cast<u8*>(basicAllocator.alloc(this->cap));
        assert(this->buffer != nullptr && "[EmbediLinearAllocator::EmbediLinearAllocator(TypeMacros::u32)] FATAL ERROR: MEMORY ALLOCATION FAILED");
        this->heapBufferAllocated = true;
    }
    else this->buffer = buf;
}

EmbediLinearAllocator::~EmbediLinearAllocator()
{
    if (this->heapBufferAllocated)
        basicAllocator.destroy(this->buffer);
}

TypeMacros::memptr EmbediLinearAllocator::alloc(TypeMacros::u32 size)
{
    if (this->buffer != nullptr)
    {
        if (this->sp + size >= this->cap)
            return nullptr;

        TypeMacros::memptr space = static_cast<u8*>(this->buffer) + this->sp;
        this->sp += size;

        return space;
    }
    else return nullptr;
}

bool EmbediLinearAllocator::destroy(TypeMacros::memptr mem)
{
    SystemIO::perror("[EmbediLinearAllocator::destroy(TypeMacros::memptr)] ERROR: destroy() method not implemented");
    return false;
}

TypeMacros::memptr EmbediLinearAllocator::reshape(TypeMacros::memptr mem, TypeMacros::u32 size)
{
    SystemIO::perror("[EmbediLinearAllocator::destroy(TypeMacros::memptr)] ERROR: reshape() method not implemented");
    return nullptr;
}

void EmbediLinearAllocator::buffer_free()
{
    this->sp = 0;
}

void EmbediLinearAllocator::dump(const char* streamName)
{
    SystemIO::fprintf(streamName, "EmbediLinearAllocator{\n\t");
    for (TypeMacros::u32 i = 0; i < __min(64UL, this->cap); i++)
        SystemIO::fprintf(streamName, "%u\n", this->buffer[i]);
    SystemIO::fprintf(streamName, "}");
}

// ------------------------------ RANDOM ACCESS MEMORY ALLOCATOR ------------------------------
inline TypeMacros::u32 read_u32(u8* ptr)
{
    return (static_cast<TypeMacros::u32>(ptr[0]) << 24) | (static_cast<TypeMacros::u32>(ptr[1]) << 16) | (static_cast<TypeMacros::u32>(ptr[2]) << 8) | (static_cast<TypeMacros::u32>(ptr[3]) & 0xFF);
}

inline void write_u32(u8* ptr, TypeMacros::u32 sz32)
{
    ptr[0] = static_cast<u8>(sz32 >> 24);        // then write allocation size
    ptr[1] = static_cast<u8>(sz32 >> 16);
    ptr[2] = static_cast<u8>(sz32 >> 8);
    ptr[3] = static_cast<u8>(sz32 & 0xFF);
}

EmbediRandomAccessMemoryAllocator::EmbediRandomAccessMemoryAllocator(u8* buf, const TypeMacros::u32 cap_size) : capacity(cap_size)
{
    if (buf == nullptr)
    {
        this->buffer = static_cast<u8*>( basicAllocator.alloc(cap_size) );
        assert(this->buffer != nullptr && "[EmbediRandomAccessMemoryAllocator::EmbediRandomAccessMemoryAllocator(u8*, TypeMacros::u32)] FATAL ERROR: FAILED TO ALLOCATE MEMORY CHUNK");
        this->heapBufferAllocated = true;
    }
    this->buffer = buf;
    
}

EmbediRandomAccessMemoryAllocator::~EmbediRandomAccessMemoryAllocator()
{
    if (this->heapBufferAllocated)
        basicAllocator.destroy(this->buffer);
}

TypeMacros::memptr EmbediRandomAccessMemoryAllocator::alloc(TypeMacros::u32 size)
{
    TypeMacros::u32 neededSpace = 1 + 4 + size;

    for (TypeMacros::u32 i = 0; i + neededSpace < this->capacity;)
    {
        if (this->buffer[i] == 0xCF)
        {
            TypeMacros::u32 allocSize = read_u32(this->buffer + i + 1);
            i += allocSize + 5;
            continue;
        }

        if (this->buffer[i] == 0)
        {
            if (i + neededSpace > this->capacity)
            {
                bool spaceFound = true;
                for (TypeMacros::u32 j = i; j < i + neededSpace; j++)
                {
                    if (this->buffer[j] == 0xCF)
                    {
                        i = j;
                        spaceFound = false;
                        break;
                    }
                }

                if (spaceFound)
                {
                    this->buffer[i++] = 0xCF;
                    write_u32(this->buffer + i, size);
                    this->objects++;
                    return (this->buffer + i + 4);
                }
            }
        }
        else i++;
    }

    return nullptr;
}

bool EmbediRandomAccessMemoryAllocator::destroy(TypeMacros::memptr mem)
{
    u8* ptr = static_cast<u8*>(mem) - 5;

    if (ptr < this->buffer || ptr > (this->buffer + this->capacity)) {
        return false;
    }

    if (*ptr != 0xCF) {
        return false;
    }

    TypeMacros::u32 alloc_size = read_u32(ptr + 1);
    memset(ptr, 0, alloc_size + 4);
    this->objects--;

    return true;
}

TypeMacros::memptr EmbediRandomAccessMemoryAllocator::reshape(TypeMacros::memptr mem, TypeMacros::u32 size)
{
    return nullptr;
}

void EmbediRandomAccessMemoryAllocator::dump(const char* streamName)
{
    SystemIO::fprintf(streamName, "Heap dump (first %u bytes) - ", 64UL);
    for (size_t i = 0; i < __min(64UL, this->capacity); i++)
    { SystemIO::fprintf(streamName, "%u ", buffer[i]); }
    SystemIO::fprintf(streamName, "\nObjects: %u\n", this->objects);
}

void EmbediRandomAccessMemoryAllocator::reset()
{
    memset(this->buffer, 0, this->capacity);
}

// ------------------------------ POOL ALLOCATOR ------------------------------
EmbediPoolAllocator::EmbediPoolAllocator(TypeMacros::memptr buffer_start, TypeMacros::u32 block_size, TypeMacros::u32 cap_size)
{
}

TypeMacros::memptr EmbediPoolAllocator::alloc(TypeMacros::u32 size)
{
    return nullptr;
}

bool  EmbediPoolAllocator::destroy(TypeMacros::memptr mem)
{
    return false;
}

TypeMacros::memptr EmbediPoolAllocator::reshape(TypeMacros::memptr mem, TypeMacros::u32 size)
{
    return nullptr;
}

void EmbediPoolAllocator::dump(const char* streamName)
{
}
