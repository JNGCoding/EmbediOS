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
    
        basicAllocator.destroy(this->blocks);
    }

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

    TypeMacros::u32 alignedPtr = align_up(this->allocPointer);

    if ((alignedPtr + size) > this->blockSize)
    {
        if (!this->arena_alloc_block())
        {
            SystemIO::perror("[EmbediArenaAllocator::alloc(TypeMacros::u32)] ERROR: FAILED TO ALLOCATE MEMORY FOR NEW BLOCK");
            return nullptr;
        }

        TypeMacros::u32 alignedPtr = align_up(this->allocPointer);
    }

    TypeMacros::memptr res = static_cast<TypeMacros::u8*>(this->blocks[this->numBlocks - 1]) + this->allocPointer;
    this->allocPointer = alignedPtr + size;

    return res;
}

bool EmbediArenaAllocator::destroy(TypeMacros::memptr mem)
{
    assert(false && "[EmbediArenaAllocator::destroy(TypeMacros::memptr)] ERROR: destroy() method not implemented");
    return false;
}

TypeMacros::memptr EmbediArenaAllocator::reshape(TypeMacros::memptr mem, TypeMacros::u32 size)
{
    assert(false && "[EmbediArenaAllocator::destroy(TypeMacros::memptr)] ERROR: reshape() method not implemented");
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
EmbediLinearAllocator::EmbediLinearAllocator(TypeMacros::u8* buf, TypeMacros::u32 cap_size) : cap(cap_size)
{
    assert(buf != nullptr && "[EmbediLinearAllocator::EmbediLinearAllocator(TypeMacros::u32)] FATAL ERROR: BUFFER PROVIDED WAS NULL");
    this->buffer = buf;
}

TypeMacros::memptr EmbediLinearAllocator::alloc(TypeMacros::u32 size)
{
    this->sp = align_up(this->sp);
    if (this->sp + size >= this->cap)
        return nullptr;

    TypeMacros::memptr space = static_cast<TypeMacros::u8*>(this->buffer) + this->sp;
    this->sp += size;

    return space;
}

bool EmbediLinearAllocator::destroy(TypeMacros::memptr mem)
{
    assert(false && "[EmbediLinearAllocator::destroy(TypeMacros::memptr)] ERROR: destroy() method not implemented");
    return false;
}

TypeMacros::memptr EmbediLinearAllocator::reshape(TypeMacros::memptr mem, TypeMacros::u32 size)
{
    assert(false && "[EmbediLinearAllocator::destroy(TypeMacros::memptr)] ERROR: reshape() method not implemented");
    return nullptr;
}

void EmbediLinearAllocator::buffer_free()
{
    this->sp = 0;
}

void EmbediLinearAllocator::dump(const char* streamName)
{
    SystemIO::fprintf(streamName, "EmbediLinearAllocator{\n\t");
    for (TypeMacros::u32 i = 0; i < embedi_min(64UL, this->cap); i++)
        SystemIO::fprintf(streamName, "%u\n", this->buffer[i]);
    SystemIO::fprintf(streamName, "}");
}
