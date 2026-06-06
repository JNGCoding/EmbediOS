#include "Arduino.h"
#include "Allocator.hpp"

// ------------------------------ BASIC ALLOCATOR ------------------------------
memptr BasicAllocator::alloc(u32 size)
{
    return malloc(size);
}

bool BasicAllocator::destroy(memptr mem)
{
    if (mem != nullptr)
    {
        free(mem);
        return true;
    }
    else return false;
}

memptr BasicAllocator::reshape(memptr mem, u32 size)
{
    memptr space = realloc(mem, size);
    if (space != nullptr)
    {
        return space;
    }
    else return nullptr;
}

void BasicAllocator::dump(FileDescriptor* file)
{
    embedi_fprintf(file, "Basic Allocator have no proper dump() method");
}

BasicAllocator basicAllocator;

// ------------------------------ ARENA ALLOCATOR ------------------------------
ArenaAllocator::ArenaAllocator(u32 _blockSize) : blockSize(_blockSize)
{
    assert(_blockSize > 0 && "[ArenaAllocator::ArenaAllocator(u32)] FATAL ERROR: BLOCK SIZE == 0, NOT SUPPORTED");
}

ArenaAllocator::~ArenaAllocator()
{
    if (this->blocks != nullptr)
    {
        for (int i = 0; i < this->numBlocks; i++)
            basicAllocator.destroy(this->blocks[i]);
    }

    basicAllocator.destroy(this->blocks);
}

bool ArenaAllocator::arena_alloc_block()
{
    if (this->blocks == nullptr)
    {
        memptr blockSpace = basicAllocator.alloc(sizeof(memptr));

        if (blockSpace == nullptr)
            return false;

        this->blocks = static_cast<memptr*>(blockSpace);
    }

    memptr space = basicAllocator.reshape(this->blocks, ++this->numBlocks * sizeof(memptr));
    if (space == nullptr)
        return false;

    memptr blockSpace = basicAllocator.reshape(this->blocks, ++this->numBlocks * sizeof(memptr));
    if (blockSpace == nullptr)
        return false;

    this->blocks = static_cast<memptr*>(blockSpace);
    this->blocks[this->numBlocks - 1] = space;
    this->allocPointer = 0;

    return true;
}

memptr ArenaAllocator::alloc(u32 size)
{
    if (this->numBlocks == 0)
    {
        if (!this->arena_alloc_block())
        {
            embedi_perror("[ArenaAllocator::alloc(u32)] ERROR: FAILED TO ALLOCATE MEMORY FOR NEW BLOCK");
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
            embedi_perror("[ArenaAllocator::alloc(u32)] ERROR: FAILED TO ALLOCATE MEMORY FOR NEW BLOCK");
            return nullptr;
        }
    }

    memptr res = static_cast<u8*>(this->blocks[this->numBlocks - 1]) + this->allocPointer;
    this->allocPointer += size;

    return res;
}

bool ArenaAllocator::destroy(memptr mem)
{
    embedi_perror("[ArenaAllocator::destroy(memptr)] ERROR: destroy() method not implemented");
    return false;
}

memptr ArenaAllocator::reshape(memptr mem, u32 size)
{
    embedi_perror("[ArenaAllocator::destroy(memptr)] ERROR: reshape() method not implemented");
    return nullptr;
}

bool ArenaAllocator::free_block(bool pseudo)
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
            memptr* _ptemp = static_cast<memptr*>(basicAllocator.reshape(this->blocks, this->numBlocks * sizeof(memptr)));
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

void ArenaAllocator::arena_free()
{
    if (this->blocks != nullptr)
    {
        for (u32 i = 0; i < this->numBlocks; i++)
            basicAllocator.destroy(this->blocks[i]);

        basicAllocator.destroy(this->blocks);
        this->blocks = nullptr;
        this->numBlocks = 0;
    }
}

void ArenaAllocator::dump(FileDescriptor *file)
{
    embedi_fprintf(file, "ArenaAllocator{\n\tblock size = %u\ntotal blocks = %u\n\n", this->blockSize, this->numBlocks);
    if (this->blocks != nullptr)
    {
        embedi_fprintf(file, "blocks:\n");
        for (u32 i = 0; i < this->numBlocks; i++)
            embedi_fprintf(file, "\tblock{%u}: %p\n", i, this->blocks[i]);
    }
    embedi_fprintf(file, "}\n");
}

// ------------------------------ LINEAR ALLOCATOR ------------------------------
LinearAllocator::LinearAllocator(u8* buf, u32 cap_size) : cap(cap_size)
{
    if (this->buffer == nullptr)
    {
        embedi_perror("[LinearAllocator::LinearAllocator(u8*, u32)] ERROR: NULLPTR PASSED, CREATING BUFFER ON BASIC ALLOCATOR");

        this->buffer = static_cast<u8*>(basicAllocator.alloc(this->cap));
        assert(this->buffer != nullptr && "[LinearAllocator::LinearAllocator(u32)] FATAL ERROR: MEMORY ALLOCATION FAILED");
    }
    else this->buffer = buf;
}

memptr LinearAllocator::alloc(u32 size)
{
    if (this->buffer != nullptr)
    {
        if (this->sp + size >= this->cap)
            return nullptr;

        memptr space = static_cast<u8*>(this->buffer) + this->sp;
        this->sp += size;

        return space;
    }
    else return nullptr;
}

bool LinearAllocator::destroy(memptr mem)
{
    embedi_perror("[LinearAllocator::destroy(memptr)] ERROR: destroy() method not implemented");
    return false;
}

memptr LinearAllocator::reshape(memptr mem, u32 size)
{
    embedi_perror("[LinearAllocator::destroy(memptr)] ERROR: reshape() method not implemented");
    return nullptr;
}

void LinearAllocator::buffer_free()
{
    this->sp = 0;
}

void LinearAllocator::dump(FileDescriptor* file)
{
    embedi_fprintf(file, "LinearAllocator{\n\t");
    for (u32 i = 0; i < min(64UL, this->cap); i++)
        embedi_fprintf(file, "%u\n", this->buffer[i]);
    embedi_fprintf(file, "}");
}

// ------------------------------ RANDOM ACCESS MEMORY ALLOCATOR ------------------------------
inline u32 read_u32(u8* ptr)
{
    return (static_cast<u32>(ptr[0]) << 24) | (static_cast<u32>(ptr[1]) << 16) | (static_cast<u32>(ptr[2]) << 8) | (static_cast<u32>(ptr[3]) & 0xFF);
}

inline void write_u32(u8* ptr, u32 sz32)
{
    ptr[0] = static_cast<u8>(sz32 >> 24);        // then write allocation size
    ptr[1] = static_cast<u8>(sz32 >> 16);
    ptr[2] = static_cast<u8>(sz32 >> 8);
    ptr[3] = static_cast<u8>(sz32 & 0xFF);
}

RandomAccessMemoryAllocator::RandomAccessMemoryAllocator(u8* buf, const u32 cap_size) : capacity(cap_size)
{
    this->buffer = buf;
}

memptr RandomAccessMemoryAllocator::alloc(u32 size)
{
    u32 neededSpace = 1 + 4 + size;

    for (u32 i = 0; i + neededSpace < this->capacity;)
    {
        if (this->buffer[i] == 0xCF)
        {
            u32 allocSize = read_u32(this->buffer + i + 1);
            i += allocSize + 5;
            continue;
        }

        if (this->buffer[i] == 0)
        {
            if (i + neededSpace > this->capacity)
            {
                bool spaceFound = true;
                for (u32 j = i; j < i + neededSpace; j++)
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

bool RandomAccessMemoryAllocator::destroy(memptr mem)
{
    u8* ptr = static_cast<u8*>(mem) - 5;

    if (ptr < this->buffer || ptr > (this->buffer + this->capacity)) {
        return false;
    }

    if (*ptr != 0xCF) {
        return false;
    }

    u32 alloc_size = read_u32(ptr + 1);
    memset(ptr, 0, alloc_size + 4);
    this->objects--;

    return true;
}

memptr RandomAccessMemoryAllocator::reshape(memptr mem, u32 size)
{
    return nullptr;
}

void RandomAccessMemoryAllocator::dump(FileDescriptor* file)
{
    embedi_fprintf(file, "Heap dump (first %u bytes) - ", 64UL);
    for (size_t i = 0; i < min(64UL, this->capacity); i++)
    { embedi_fprintf(file, "%u ", buffer[i]); }
    embedi_fprintf(file, "\nObjects: %u\n", this->objects);
}

void RandomAccessMemoryAllocator::reset()
{
    memset(this->buffer, 0, this->capacity);
}
