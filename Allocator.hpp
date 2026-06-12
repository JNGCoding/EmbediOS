#ifndef __EMBEDI_ALLOCATOR_HPP__
#define __EMBEDI_ALLOCATOR_HPP__

#include <Arduino.h>
#include "TypeMacros.hpp"
#include "IOStreams.hpp"
#include <stdlib.h>

#ifndef __AVR__
#include <assert.h>
#else
#define assert(expr) ((expr) ? (void)0 : abort())
#endif

struct EmbediAllocator
{
    virtual ~EmbediAllocator() = default;
    
    virtual TypeMacros::memptr alloc(TypeMacros::u32 size)
    { return nullptr; }
    
    virtual bool  destroy(TypeMacros::memptr mem)
    { return true; }
    
    virtual TypeMacros::memptr reshape(TypeMacros::memptr mem, TypeMacros::u32 size)
    { return nullptr; }

    virtual void dump(const char* streamName)
    { SystemIO::fprintf(streamName, "Allocator interface have no proper dump() method"); }
};

struct EmbediBasicAllocator : public EmbediAllocator
{
    TypeMacros::memptr alloc(TypeMacros::u32 size) override;
    bool  destroy(TypeMacros::memptr mem) override;
    TypeMacros::memptr reshape(TypeMacros::memptr mem, TypeMacros::u32 size) override;
    void dump(const char* streamName) override;
};

extern EmbediBasicAllocator basicAllocator;

// TODO: Input an allocator parameter to this allocator
// So that it can allocate from a pre-allocated buffer instead from the system
// directly
class EmbediArenaAllocator : public EmbediAllocator
{
private:
    const TypeMacros::u32 blockSize = 0;
    TypeMacros::u32 numBlocks       = 0;
    TypeMacros::u32 allocPointer    = 0;
    TypeMacros::memptr* blocks      = nullptr;
    bool arena_alloc_block();

public:
    EmbediArenaAllocator(TypeMacros::u32 _blockSize);
    ~EmbediArenaAllocator();
    TypeMacros::memptr alloc(TypeMacros::u32 size) override;
    bool  destroy(TypeMacros::memptr mem) override;
    TypeMacros::memptr reshape(TypeMacros::memptr mem, TypeMacros::u32 size) override;
    bool free_block(bool pseudo = false);
    void arena_free();
    TypeMacros::u32 get_current_allocated_blocks() const { return this->numBlocks; }
    TypeMacros::u32 get_current_allocated_memory() const { return this->numBlocks * this->blockSize; }
    void dump(const char* streamName) override;
};

class EmbediLinearAllocator : public EmbediAllocator
{
private:
    u8* buffer    = nullptr;
    TypeMacros::u32 sp        = 0;
    const TypeMacros::u32 cap = 0;
    bool heapBufferAllocated = false;

public:
    EmbediLinearAllocator(u8* buf, TypeMacros::u32 cap_size);
    ~EmbediLinearAllocator();
    TypeMacros::memptr alloc(TypeMacros::u32 size) override;
    bool  destroy(TypeMacros::memptr mem) override;
    TypeMacros::memptr reshape(TypeMacros::memptr mem, TypeMacros::u32 size) override;
    void buffer_free();
    void dump(const char* streamName) override;
};

class EmbediRandomAccessMemoryAllocator : public EmbediAllocator
{
private:
    const TypeMacros::u32 capacity = 0;
    TypeMacros::u32 objects = 0;
    u8* buffer = nullptr;
    bool heapBufferAllocated = false;

public:
    EmbediRandomAccessMemoryAllocator(u8* buf, const TypeMacros::u32 cap_size);
    ~EmbediRandomAccessMemoryAllocator();
    TypeMacros::memptr alloc(TypeMacros::u32 size) override;
    bool destroy(TypeMacros::memptr mem) override;
    TypeMacros::memptr reshape(TypeMacros::memptr mem, TypeMacros::u32 size) override;
    void dump(const char* streamName) override;
    void reset();
};

class EmbediPoolAllocator : public EmbediAllocator
{
private:
    struct {
        bool free;
        TypeMacros::memptr memory;
    } *buffer;

public:
    EmbediPoolAllocator(TypeMacros::memptr buffer_start, TypeMacros::u32 block_size, TypeMacros::u32 cap_size);
    TypeMacros::memptr alloc(TypeMacros::u32 size) override;
    bool  destroy(TypeMacros::memptr mem) override;
    TypeMacros::memptr reshape(TypeMacros::memptr mem, TypeMacros::u32 size) override;
    void dump(const char* streamName) override;
};

template<TypeMacros::u32 bufferSize, TypeMacros::u32 heapAllocedSize, TypeMacros::u32 heapFreedSize>
class EmbediHeapAllocator : public EmbediAllocator
{
private:
    struct HeapChunk
    {
        TypeMacros::memptr ptr;
        TypeMacros::u32 memsize;
    };

    u8 buffer[bufferSize] = {0};
    HeapChunk chunks[heapAllocedSize];
    HeapChunk freedChunks[heapFreedSize];

    TypeMacros::u32 heapSize = 0;
    TypeMacros::u32 heapAllocSize = 0;

public:
    TypeMacros::memptr alloc(TypeMacros::u32 size) override;
    bool  destroy(TypeMacros::memptr mem) override;
    TypeMacros::memptr reshape(TypeMacros::memptr mem, TypeMacros::u32 size) override;
    void dump(const char* streamName) override;
};

#endif