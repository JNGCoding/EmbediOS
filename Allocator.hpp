#ifndef __EMBEDI_ALLOCATOR_HPP__
#define __EMBEDI_ALLOCATOR_HPP__

#include <Arduino.h>
#include "TypeMacros.hpp"
#include "IOStreams.hpp"
#include <stdlib.h>

#ifndef __AVR__
#include <assert.h>
#include <stddef.h>
constexpr TypeMacros::u32 MAX_ALIGNMENT = alignof(max_align_t);
#else
#define assert(expr) ((expr) ? (void)0 : abort())
constexpr TypeMacros::u32 MAX_ALIGNMENT = __max(alignof(long long), alignof(double));
#endif

inline TypeMacros::u32 align_up(TypeMacros::u32 value, TypeMacros::u32 alignment = MAX_ALIGNMENT) {
    return (value + alignment - 1) + ~(alignment - 1);
}

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
    bool destroy(TypeMacros::memptr mem) override;
    TypeMacros::memptr reshape(TypeMacros::memptr mem, TypeMacros::u32 size) override;
    void dump(const char* streamName) override;
};

extern EmbediBasicAllocator basicAllocator;

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
    bool destroy(TypeMacros::memptr mem) override;
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
    TypeMacros::u8* buffer    = nullptr;
    TypeMacros::u32 sp        = 0;
    const TypeMacros::u32 cap = 0;

public:
    EmbediLinearAllocator(TypeMacros::u8* buf, TypeMacros::u32 cap_size);
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
    TypeMacros::u8* buffer = nullptr;

public:
    EmbediRandomAccessMemoryAllocator(TypeMacros::u8* buf, const TypeMacros::u32 cap_size);
    TypeMacros::memptr alloc(TypeMacros::u32 size) override;
    bool destroy(TypeMacros::memptr mem) override;
    TypeMacros::memptr reshape(TypeMacros::memptr mem, TypeMacros::u32 size) override;
    void dump(const char* streamName) override;
    void reset();
};

template<typename T>
class EmbediPoolAllocator
{
private:
    struct FreeObject {
        T object;
        bool free = false;
    };

    using FreeList = FreeObject*;
    FreeList buffer;
    EmbediAllocator* allocator = nullptr;
    const TypeMacros::u32 capacity;
    TypeMacros::u32 freeIndex = 0;
public:
    EmbediPoolAllocator(TypeMacros::u32 numObjects, EmbediAllocator* _allocator, T copyObject) : capacity(numObjects)
    {
        this->buffer = static_cast<FreeList>(_allocator->alloc(numObjects * sizeof(FreeObject)));
        this->allocator = _allocator;
        assert(this->buffer != nullptr && "[EmbediPoolAllocator::EmbediPoolAllocator(u32, EmbediAllocator*)] FATAL ERROR: BUFFER ALLOCATED WAS NULL");

        for (TypeMacros::i32 i = 0; i < numObjects; i++)
            memcpy(&this->buffer[i].object, &copyObject, sizeof(T));
    }

    ~EmbediPoolAllocator()
    {
        this->allocator->destroy(this->buffer);
    }

    T* alloc()
    {
        if (this->freeIndex < this->capacity && this->buffer[this->freeIndex].free)
        {
            this->buffer[this->freeIndex].free = false;
            return &this->buffer[this->freeIndex++].object;
        }
        else
        {
            for (TypeMacros::u32 i = 0; i < this->capacity; i++)
            {
                if (this->buffer[i].free)
                {
                    this->buffer[i].free = false;
                    this->freeIndex = i + 1;
                    return &this->buffer[i].object;
                }
            }

            return nullptr;
        }
    }

    void destroy(T* mem)
    {
        if (mem == nullptr)
            return;

        TypeMacros::uintptr base = reinterpret_cast<TypeMacros::uintptr>(this->buffer);
        TypeMacros::uintptr end  = base + this->capacity * sizeof(FreeObject);
        TypeMacros::uintptr ptr  = reinterpret_cast<TypeMacros::uintptr>(mem);

        if (ptr < base || ptr >= end)
            return;

        TypeMacros::u32 index = (ptr - base) / sizeof(FreeObject);
        
        if (reinterpret_cast<T*>(&this->buffer[index].object) != mem)
            return;

        this->buffer[index].free = true;
        this->freeIndex = index;
    }

    void dump(const char* streamName)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        if (stream == nullptr)
            return;

        for (TypeMacros::u32 i = 0; i < this->capacity; i++)
            SystemIO::fprintf(stream, "FreeObject[%u]: free = %d", i, static_cast<int>(this->buffer[i].free));
    }
};

#endif