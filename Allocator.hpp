#ifndef __EMBEDI_ALLOCATOR_HPP__
#define __EMBEDI_ALLOCATOR_HPP__

#include "TypeMacros.hpp"
#include "IOStreams.hpp"
#include <stdlib.h>

struct Allocator
{
    virtual memptr alloc(u32 size)
    { return nullptr; }
    
    virtual bool  destroy(memptr mem)
    { return true; }
    
    virtual memptr reshape(memptr mem, u32 size)
    { return nullptr; }

    virtual void dump(FileDescriptor* file)
    { embedi_fprintf(file, "Allocator interface have no proper dump() method"); }
};

struct BasicAllocator : public Allocator
{
    memptr alloc(u32 size) override;
    bool  destroy(memptr mem) override;
    memptr reshape(memptr mem, u32 size) override;
    void dump(FileDescriptor* file) override;
};

extern BasicAllocator basicAllocator;

class ArenaAllocator : public Allocator
{
private:
    const u32 blockSize = 0;
    u32 numBlocks       = 0;
    u32 allocPointer    = 0;
    memptr* blocks       = nullptr;
    bool arena_alloc_block();

public:
    ArenaAllocator(u32 _blockSize);
    ~ArenaAllocator();
    memptr alloc(u32 size) override;
    bool  destroy(memptr mem) override;
    memptr reshape(memptr mem, u32 size) override;
    bool free_block(bool pseudo = false);
    void arena_free();
    const u32 get_current_allocated_blocks() const { return this->numBlocks; }
    const u32 get_current_allocated_memory() const { return this->numBlocks * this->blockSize; }
    void dump(FileDescriptor* file) override;
};

class LinearAllocator : public Allocator
{
private:
    u8* buffer    = nullptr;
    u32 sp        = 0;
    const u32 cap = 0;

public:
    LinearAllocator(u8* buf, u32 cap_size);
    memptr alloc(u32 size) override;
    bool  destroy(memptr mem) override;
    memptr reshape(memptr mem, u32 size) override;
    void buffer_free();
    void dump(FileDescriptor* file) override;
};

class RandomAccessMemoryAllocator : public Allocator
{
private:
    const u32 capacity = 0;
    u32 objects = 0;
    u8* buffer = nullptr;

public:
    RandomAccessMemoryAllocator(u8* buf, const u32 cap_size);
    memptr alloc(u32 size) override;
    bool destroy(memptr mem) override;
    memptr reshape(memptr mem, u32 size) override;
    void dump(FileDescriptor* file) override;
    void reset();
};

// class PoolAllocator : public Allocator
// {
// private:
//     struct {
//         bool free;
//         memptr memory;
//     } *buffer;

// public:
//     PoolAllocator(u32 block_size, u32 cap_size);
//     memptr alloc(u32 size) override;
//     bool  destroy(memptr mem) override;
//     memptr reshape(memptr mem, u32 size) override;
//     void dump(FileDescriptor* file) override;
// };

#endif