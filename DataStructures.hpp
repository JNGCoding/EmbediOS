#ifndef __EMBEDI_DATA_STRUCTURES_HPP__
#define __EMBEDI_DATA_STRUCTURES_HPP__

#include <Arduino.h>
#include "TypeMacros.hpp"
#include "IOStreams.hpp"
#include "Allocator.hpp"
#include <string.h>

#ifndef __AVR__

// AVR Compilers doesn't define assert.h headers
// Currently, they are the only compilers that I know of that don't have assert.h
#include <assert.h>

#else

#define assert(expr) ((expr) ? (void)0 : abort())

#endif

// A simple pair class
template<typename T1, typename T2>
struct EmbediPair
{
    T1 a;
    T2 b;

    void set(T1 o1, T2 o2)
    {
        a = o1;
        b = o2;
    }

    bool equals(const EmbediPair<T1, T2> other)
    {
        return a == other.a && b == other.b;
    }
};

// A simple singly-linked node
template<typename T>
struct SinglyNode
{
    T value;
    SinglyNode<T>* root = nullptr;
};

// A simple doubly-linked node
template<typename T>
struct DoublyNode
{
    DoublyNode<T>* root = nullptr;
    DoublyNode<T>* head = nullptr;
    T value;
};

// Generic Dynamically allocated String type
class EmbediString
{
private:
    EmbediAllocator* allocator = nullptr;
    char* data = nullptr;
    TypeMacros::u32 capacity = 0;
    TypeMacros::u32 len = 0;

public:
    constexpr static TypeMacros::f32 MULTIPLY_FACTOR = 2.0f;

    // Construct an empty string with the given allocator (defaults to basicAllocator)
    // `_allocator`: the allocator which will be used for allocations
    explicit EmbediString(EmbediAllocator* _allocator = &basicAllocator);

    // Construct from a C-style string, copying its contents into managed memory
    // `_data`: the data which will be copied
    // `_allocator`: the allocator which will be used for allocations
    explicit EmbediString(const char* _data, EmbediAllocator* _allocator = &basicAllocator);

    // Copy constructor: creates a new string by copying another EmbediString
    // If allocator is provided, uses that; otherwise inherits from source string
    // `str`: the EmbediString object which will be copied from
    // `_allocator`: preferred allocator for allocations of this object
    explicit EmbediString(const EmbediString& str, EmbediAllocator* _allocator = nullptr);

    // Destructor: releases allocated memory
    // warning: if allocator doesn't support destroy() operations then memory will not be deallocated
    ~EmbediString();

    // Finds a first occurence `substr` in the object's data buffer and return its index
    // if not found, then returns -1
    // `substr`: the substring sequence to search for
    TypeMacros::i64 find(const char* substr);

    // Finds a first occurence of the contents of `substr` in the object's data buffer and return its index
    // if not found, then returns -1
    // `substr`: the EmbediString instance whose contents to search for
    TypeMacros::i64 find(const EmbediString& substr);

    // Stores the substring of this object into other EmbediString object
    // `buffer` the EmbediString object we will write the substring into
    // `start` start index of the substring
    // `end` end index of the substring
    bool get_substring(EmbediString& buffer, const TypeMacros::u32 start, const TypeMacros::u32 end);

    // Returns the pointer to the character at the index provided
    // `index` index of the character
    char* get_char(const TypeMacros::u32 index);

    bool contains(const char* substr);

    bool contains(const EmbediString& substr);

    bool is_blank();

    bool is_digit();

    bool is_ascii();

    bool is_nothing();

    void lower();

    void upper();

    bool append(const char* str);

    bool append(const char* str, const TypeMacros::u32 start);

    bool append(const char* str, const TypeMacros::u32 start, const TypeMacros::u32 end);

    bool append(const EmbediString& str);

    bool append(const EmbediString& str, const TypeMacros::u32 start);

    bool append(const EmbediString& str, const TypeMacros::u32 start, const TypeMacros::u32 end);

    bool replace(const char* old, const char* sequence);

    void clear();

    void shrink_to_size();

    const char* c_str();

    TypeMacros::u32 length() const;

    bool equals(const EmbediString& other);
};

// Generic Doubly linked List class
template<typename T>
class EmbediList
{
private:
    using ListNode = DoublyNode<T>;

    ListNode* tail = nullptr;
    ListNode* head = nullptr;
    EmbediAllocator* allocator = nullptr;
    TypeMacros::u32 count = 0;

public:
    explicit EmbediList(EmbediAllocator* _allocator = &basicAllocator)
    {
        assert(_allocator != nullptr && "[EmbediList::EmbediList()] FATAL ERROR: ALLOCATOR WAS NULL");
        this->allocator = _allocator;
    }

    TypeMacros::u32 copy(EmbediList& other)
    {
        ListNode* other_cur = other.tail;
        TypeMacros::u32 elements_added = 0;

        while (other_cur->head != nullptr) {
            if (!this->push(other_cur->value))
                return elements_added;

            other_cur = other_cur->head;
            elements_added++;
        }

        return elements_added + (this->push(other_cur->value) ? 1 : 0);
    }

    ~EmbediList()
    {
        this->clear();
    }

    T* get(const TypeMacros::u32 index)
    {
        if (index >= this->count)
            return nullptr;

        ListNode* cur = this->head;

        if (index >= static_cast<TypeMacros::u32>(this->count / 2))
        {
            TypeMacros::u32 _index = this->count - index - 1;
            for (TypeMacros::u32 i = 0; i < _index; i++)
                cur = cur->root;
        }
        else
        {
            cur = this->tail;
            for (TypeMacros::u32 i = 0; i < index; i++)
                cur = cur->head;
        }

        T* result = &cur->value;

        return result;
    }

    void set(const T& value, const TypeMacros::u32 index)
    {
        if (index >= this->count) return;

        ListNode* cur = this->head;

        if (index >= static_cast<TypeMacros::u32>(this->count / 2))
        {
            TypeMacros::u32 _index = this->count - index - 1;
            for (TypeMacros::u32 i = 0; i < _index; i++)
                cur = cur->root;
        }
        else
        {
            cur = this->tail;
            for (TypeMacros::u32 i = 0; i < index; i++)
                cur = cur->head;
        }

        cur->value = value;
    }

    bool append(const T& element)
    {
        if (this->head == nullptr)
        {
            this->head = this->tail = static_cast<ListNode*>( this->allocator->alloc(sizeof(ListNode)) );
            if (this->head == nullptr)
                return false;

            this->head->value = element;
            this->head->head = nullptr;
            this->head->root = nullptr;
            this->count++;
        }
        else
        {
            ListNode* n = static_cast<ListNode*>(this->allocator->alloc(sizeof(ListNode)));
            if (n == nullptr)
                return false;

            n->root = this->head;
            n->value = element;
            n->head = nullptr;

            this->head->head = n;
            this->head = n;

            this->count++;
        }

        return true;
    }

    bool insert(const T& element, const TypeMacros::u32 index)
    {
        if (index >= this->count) return false;

        // Create the new node
        ListNode* n = static_cast<ListNode*>(this->allocator->alloc(sizeof(ListNode)));
        if (n == nullptr)
            return false;

        n->root = nullptr;
        n->value = element;
        n->head = nullptr;

        if (index == 0)
        {
            n->head = this->tail;
            this->tail->root = n;
            this->tail = n;
            this->count++;

            return true;
        }

        if (index == this->count - 1)
        {
            n->root = this->head;
            this->head->head = n;
            this->head = n;
            this->count++;
            return true;
        }

        ListNode* cur = this->head;
        if (index >= static_cast<TypeMacros::u32>(this->count / 2))
        {
            TypeMacros::u32 _index = this->count - index - 1;
            for (TypeMacros::u32 i = 0; i < _index; i++)
                cur = cur->root;
        }
        else
        {
            cur = this->tail;
            for (TypeMacros::u32 i = 0; i < index; i++)
                cur = cur->head;
        }

        ListNode* previous_node = cur->root;

        previous_node->head = n;
        n->root = previous_node;
        cur->root = n;
        n->head = cur;

        this->count++;

        return true;
    }

    bool remove(const TypeMacros::u32 index)
    {
        if (this->count == 0 || index >= this->count)
            return false;

        ListNode* cur = this->head;
        if (index >= static_cast<TypeMacros::u32>(this->count / 2))
        {
            TypeMacros::u32 _index = this->count - index - 1;
            for (TypeMacros::u32 i = 0; i < _index; i++)
                cur = cur->root;
        }
        else
        {
            cur = this->tail;
            for (TypeMacros::u32 i = 0; i < index; i++)
                cur = cur->head;
        }

        ListNode* removal_node = cur;
        ListNode* previous_node = cur->root;
        ListNode* next_node = cur->head;

        if (previous_node == nullptr && next_node == nullptr)
        {
            this->allocator->destroy(removal_node);
            this->head = nullptr;
            this->tail = nullptr;
            this->count = 0;

            return true;
        }

        if (previous_node == nullptr)
        {
            this->tail = this->tail->head;
            this->tail->root = nullptr;
            this->allocator->destroy(removal_node);
            this->count--;

            return true;
        }

        if (next_node == nullptr)
        {
            this->head = this->head->root;
            this->head->head = nullptr;
            this->allocator->destroy(removal_node);
            this->count--;
            return true;
        }

        previous_node->head = next_node;
        next_node->root = previous_node;
        this->allocator->destroy(removal_node);

        this->count--;

        return true;
    }

    void clear()
    {
        while (this->count > 0)
            this->remove(this->count - 1);
    }

    bool equals(EmbediList& other)
    {
        if (this->size() != other.count)
            return false;

        bool flag = true;

        ListNode* this_cur = this->tail;
        ListNode* other_cur = other.tail;

        for (TypeMacros::u32 i = 0; i < this->size(); i++)
        {
            if (this_cur->value != other_cur->value)
            {
                flag = false;
                break;
            }

            this_cur = this_cur->head;
            other_cur = other_cur->head;
        }

        return flag;
    }

    TypeMacros::u32 size() const
    {
        return this->count;
    }

    ListNode* get_tail()
    {
        return this->tail;
    }

    ListNode* get_head()
    {
        return this->head;
    }
};

// Generic fixed-capacity buffer Stack data structure
template<typename T>
class EmbediStack
{
private:
    T* buffer = nullptr;
    TypeMacros::u32 count = 0;
    const TypeMacros::u32 capacity;
    bool heapBufferAllocated = false;

public:
    explicit EmbediStack(T* buf, const TypeMacros::u32 cap_size) : capacity(cap_size)
    {
        if (buf == nullptr)
        {
            this->buffer = static_cast<T*>(basicAllocator.alloc(sizeof(T) * cap_size));
            assert(this->buffer != nullptr && "[EmbediStack::EmbediStack(T*, TypeMacros::u32)] FATAL ERROR: FAILED TO ALLOCATE BUFFER ON BASIC ALLOCATOR");
            this->heapBufferAllocated = true;
        }
        else this->buffer = buf;
    }

    TypeMacros::u32 copy(EmbediStack& other, const TypeMacros::u32 cpy_size) const
    {
        this->clear();

        for (TypeMacros::u32 i = 0; i < cpy_size && i < other.count && i < this->count; i++)
        {
            if (!this->push(other.buffer[i]))
                return i - 1;
        }

        return cpy_size;
    }

    ~EmbediStack()
    {
        if (this->heapBufferAllocated)
            basicAllocator.destroy(this->buffer);
    }

    bool push(const T& element)
    {
        if (this->count >= this->capacity)
            return false;

        this->buffer[this->count++] = element;
        return true;
    }

    T* pop()
    {
        if (this->count <= 0)
            return nullptr;

        return &this->buffer[(this->count--) - 1];
    }

    T* peek()
    {
        if (this->count <= 0)
            return nullptr;

        return &this->buffer[this->count - 1];
    }

    void clear()
    {
        this->count = 0;
    }

    TypeMacros::u32 size() const
    {
        return this->count;
    }

    const T* get_buffer() const
    {
        return this->buffer;
    }

    bool equals(const EmbediStack& other)
    {
        if (this->count != other.count)
            return false;

        for (TypeMacros::u32 i = 0; i < this->count; i++)
        {
            if (this->buffer[i] != other.buffer[i])
                return false;
        }

        return true;
    }
};

// Generic Vector-style Dynamic Stack Data Structure
template<typename T>
class EmbediDynamicStack
{
private:
    T* buffer = nullptr;
    EmbediAllocator* allocator = nullptr;
    TypeMacros::u32 count = 0;
    TypeMacros::u32 capacity = 0;

public:
    constexpr static TypeMacros::f32 MULTIPLY_FACTOR = 2.0f;
    constexpr static TypeMacros::u32 PRE_ALLOCATE_DURING_CONSTRUCTION = 10;

    explicit EmbediDynamicStack(EmbediAllocator* _allocator = &basicAllocator)
    {
        assert(_allocator != nullptr && "[EmbediDynamicStack::EmbediDynamicStack()] FATAL ERROR: ALLOCATOR WAS NULL");

        this->allocator = _allocator;
        this->buffer = static_cast<T*>(this->allocator->alloc(sizeof(T) * EmbediDynamicStack::PRE_ALLOCATE_DURING_CONSTRUCTION));

        if (this->buffer == nullptr)
        {
            SystemIO::perror("[EmbediDynamicStack::EmbediDynamicStack()] ERROR: FAILED TO ALLOCATE MEMORY FROM ALLOCATOR");
            return;
        }

        this->capacity = EmbediDynamicStack::PRE_ALLOCATE_DURING_CONSTRUCTION;
    }

    TypeMacros::u32 copy(EmbediDynamicStack& other, const TypeMacros::u32 cpy_size)
    {
        this->clear();

        TypeMacros::u32 elements_added = 0;

        for (int i = 0; i < other.size() && i < cpy_size; i++)
        {
            if (!this->push(other.buffer[i]))
                return elements_added;

            elements_added++;
        }

        return elements_added;
    }

    ~EmbediDynamicStack()
    {
        this->allocator->destroy(this->buffer);
    }

    bool push(const T& element)
    {
        if (this->count >= this->capacity)
        {
            T* space = static_cast<T*>(this->allocator->reshape(this->buffer, sizeof(T) * (this->capacity * EmbediDynamicStack::MULTIPLY_FACTOR)));
            if (space == nullptr)
                return false;

            this->buffer = space;
            this->capacity *= EmbediDynamicStack::MULTIPLY_FACTOR;
        }

        this->buffer[this->count++] = element;
        return true;
    }

    T* pop()
    {
        if (this->count <= 0)
            return nullptr;

        return &this->buffer[(this->count--) - 1];
    }

    T* peek()
    {
        if (this->count <= 0)
            return nullptr;

        return &this->buffer[this->count - 1];
    }

    void clear()
    {
        this->count = 0;
    }

    bool equals(EmbediDynamicStack& other)
    {
        if (this->count != other.count)
            return false;

        for (TypeMacros::u32 i = 0; i < this->count; i++)
        {
            if (this->buffer[i] != other.buffer[i])
                return false;
        }

        return true;
    }

    TypeMacros::u32 size() const
    {
        return this->count;
    }

    TypeMacros::u32 cap_size() const
    {
        return this->capacity;
    }

    void shrink_to_size()
    {
        if (this->capacity == this->count)
            return;

        char* space = static_cast<T*>(this->allocator->reshape(this->buffer, this->count * sizeof(T)));
        if (space == nullptr)
            return;

        this->data = space;
    }

    const T* get_buffer() const
    {
        return this->buffer;
    }
};

// Generic fixed-capacity RingBuffer Data Structure
template<typename T>
class EmbediRingBuffer
{
private:
    T* buffer = nullptr;
    const TypeMacros::u32 capacity;

    TypeMacros::u32 appendPtr = 0;
    TypeMacros::u32 readPtr = 0;
    TypeMacros::u32 count = 0;

    bool heapBufferAllocated = false;

public:
    explicit EmbediRingBuffer(T* buf, const TypeMacros::u32 cap_size) : capacity(cap_size)
    {
        if (buf == nullptr)
        {
            this->buffer = static_cast<T*>(basicAllocator.alloc(sizeof(T) * cap_size));
            this->heapBufferAllocated = true;
            assert(this->buffer != nullptr && "[EmbediRingBuffer::EmbediRingBuffer(T*, TypeMacros::u32)] FATAL ERROR: FAILED TO ALLOCATE BUFFER ON BASIC ALLOCATOR");
        }
        else this->buffer = buf;
    }

    TypeMacros::u32 copy(const EmbediRingBuffer& other)
    {
        this->clear();

        if (this->count != other.count)
            return 0;

        memcpy(this->buffer, other.buffer, sizeof(T) * other.count);

        return other.count;
    }

    ~EmbediRingBuffer()
    {
        if (this->heapBufferAllocated)
            basicAllocator.destroy(this->buffer);
    }

    void push(const T& element)
    {
        this->buffer[this->appendPtr++] = element;
        this->appendPtr %= this->capacity;

        this->count++;
        if (this->count >= this->capacity)
            this->count = this->capacity;
    }

    T* pop()
    {
        if (this->count <= 0)
            return nullptr;

        T* result = &this->buffer[this->readPtr++];
        this->readPtr %= this->capacity;
        this->count--;

        return result;
    }

    void clear()
    {
        this->appendPtr = this->readPtr;
        this->count = 0;
    }

    TypeMacros::u32 size() const
    {
        return this->count;
    }
};

template<typename T>
class EmbediVector
{
private:
    T* buffer = nullptr;
    TypeMacros::u32 count = 0;
    TypeMacros::u32 capacity = 0;
    EmbediAllocator* allocator = nullptr;

public:
    constexpr static TypeMacros::f32 MULTIPLY_FACTOR = 2.0f;
    constexpr static TypeMacros::u32 PRE_ALLOCATE_DURING_CONSTRUCTION = 10;

    EmbediVector(EmbediAllocator* _allocator = &basicAllocator)
    {
        assert(_allocator != nullptr && "[EmbediVector::EmbediVector()] FATAL ERROR: ALLOCATOR WAS NULL");

        this->allocator = _allocator;
        this->buffer = static_cast<T*>(this->allocator->alloc(sizeof(T) * EmbediVector::PRE_ALLOCATE_DURING_CONSTRUCTION));

        if (this->buffer == nullptr)
        {
            SystemIO::perror("[EmbediVector::EmbediVector()] ERROR: FAILED TO ALLOCATE MEMORY FROM ALLOCATOR");
            return;
        }

        this->capacity = EmbediVector::PRE_ALLOCATE_DURING_CONSTRUCTION;
    }

    ~EmbediVector()
    {
        if (this->buffer != nullptr)
            this->allocator->destroy(this->buffer);
    }

    TypeMacros::u32 copy(const EmbediVector& other, const TypeMacros::u32 cpy_size)
    {
        this->clear();

        TypeMacros::u32 elements_added = 0;

        for (int i = 0; i < other.size() && i < cpy_size; i++)
        {
            if (!this->push(other.buffer[i]))
                return elements_added;

            elements_added++;
        }

        return elements_added;
    }

    bool add(const T& value)
    {
        if (this->count >= this->capacity)
        {
            T* space = static_cast<T*>(this->allocator->reshape(this->buffer, sizeof(T) * (this->capacity * EmbediVector::MULTIPLY_FACTOR)));
            if (space == nullptr)
                return false;

            this->buffer = space;
            this->capacity *= EmbediVector::MULTIPLY_FACTOR;
        }

        this->buffer[this->count++] = value;
        return true;
    }

    bool insert(const T& value, const TypeMacros::u32 index)
    {
        if (index >= this->count)
            return false;

        if (this->count >= this->capacity)
        {
            T* space = static_cast<T*>(this->allocator->reshape(this->buffer, sizeof(T) * (this->capacity * EmbediVector::MULTIPLY_FACTOR)));
            if (space == nullptr)
                return false;

            this->buffer = space;
            this->capacity *= EmbediVector::MULTIPLY_FACTOR;
        }

        for (TypeMacros::u32 i = this->count - 1; i > index; i--)
            this->buffer[i] = this->buffer[i - 1];

        this->buffer[index] = value;
        this->count++;

        return true;
    }

    bool remove(const TypeMacros::u32 index)
    {
        if (index >= this->count)
            return false;

        for (TypeMacros::u32 i = index; i < this->count - 1; i++)
            this->buffer[i] = this->buffer[i + 1];
        
        this->count--;

        return true;
    }

    T* pop()
    {
        if (this->count <= 0)
            return nullptr;

        T* result = this->buffer[this->count - 1];
        this->remove(this->count - 1);
        return result;
    }

    T* get(const TypeMacros::u32 index)
    {
        if (index >= this->count)
            return nullptr;

        return &this->buffer[index];
    }

    bool set(const T& value, const TypeMacros::u32 index)
    {
        if (index >= this->count)
            return nullptr;

        this->buffer[index] = value;

        return false;
    }

    void clear()
    {
        this->count = 0;
    }

    void shrink_to_size()
    {
        if (this->count == this->capacity)
            return;

        T* space = static_cast<T*>(this->allocator->reshape(this->buffer, this->count * sizeof(T)));
        if (space == nullptr)
            return;

        this->buffer = space;
    }

    TypeMacros::u32 size() const
    {
        return this->count;
    }

    TypeMacros::u32 cap_size() const
    {
        return this->capacity;
    }
};

// Generic HASH_FUNCTION
template<typename K>
TypeMacros::usize EMBEDI_GENERIC_HASH_FUNCTION(K value)
{ return reinterpret_cast<TypeMacros::usize>(value); }

// HASHING FUNCTION SPECIALIZATIONS

template<>
inline TypeMacros::usize EMBEDI_GENERIC_HASH_FUNCTION<const char*>(const char* key)
{
    constexpr TypeMacros::usize FNV_offset_basis = 0xCBF29CE484222325;
    constexpr TypeMacros::usize FNV_prime = 1099511628211ULL;

    TypeMacros::usize hash = FNV_offset_basis;

    TypeMacros::u32 strsize = strlen(key);

    for (TypeMacros::u32 i = 0; i < strsize; i++)
    {
        hash = hash ^ key[i];
        hash = hash * FNV_prime;
    }

    return hash;
}

template<>
inline TypeMacros::usize EMBEDI_GENERIC_HASH_FUNCTION<int>(int key)
{ return static_cast<TypeMacros::usize>(key); }

template<>
inline TypeMacros::usize EMBEDI_GENERIC_HASH_FUNCTION<unsigned int>(unsigned int key)
{ return static_cast<TypeMacros::usize>(key); }

template<>
inline TypeMacros::usize EMBEDI_GENERIC_HASH_FUNCTION<double>(double key)
{
    union { double d; uint64_t u64; } conv;
    conv.d = key;

    TypeMacros::usize x = conv.u64;

    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;

    return static_cast<TypeMacros::usize>(x);
}

template<>
inline TypeMacros::usize EMBEDI_GENERIC_HASH_FUNCTION<float>(float key)
{
    union { float f; uint32_t u32; } conv;
    conv.f = key;

    TypeMacros::u32 x = conv.u32;

    x ^= x >> 16;
    x *= 0x85ebca6bU;
    x ^= x >> 13;
    x *= 0xc2b2ae35U;
    x ^= x >> 16;

    return static_cast<TypeMacros::usize>(x);
}

template<>
inline TypeMacros::usize EMBEDI_GENERIC_HASH_FUNCTION<TypeMacros::memptr>(TypeMacros::memptr key)
{ return reinterpret_cast<TypeMacros::usize>(key); }

template<typename K, typename V>
class EmbediHashTable
{
private:
    using Entry = EmbediPair<K, V>;
    EmbediList<Entry>* buckets = nullptr;
    const TypeMacros::u32 numBuckets;
    TypeMacros::u32 count = 0;

public:
    constexpr static TypeMacros::u32 DEFAULT_NUM_BUCKETS = 10;

    explicit EmbediHashTable(const TypeMacros::u32 _numBuckets = EmbediHashTable::DEFAULT_NUM_BUCKETS, EmbediAllocator* _allocator = &basicAllocator)
        : numBuckets(_numBuckets)
    {
        assert(_allocator != nullptr && "[EmbediHashTable::EmbediHashTable()] FATAL ERROR: ALLOCATOR WAS NULL");

        this->buckets = basicAllocator.alloc( sizeof(EmbediList<Entry>*) * _numBuckets );
        if (this->buckets == nullptr)
        {
            SystemIO::perror("[EmbediHashTable::EmbediHashTable()] ERROR: FAILED TO ALLOCATE BUCKETS FOR HASHTABLE. IMPLICATIONS IMMINENT");
            return;
        }

        // A default list with non - trivial components or whatever for copying
        EmbediList<Entry> cpy_list(_allocator);

        for (TypeMacros::u32 i = 0; i < this->numBuckets; i++)
        {
            EmbediList<Entry>* bucket = basicAllocator.alloc( sizeof(EmbediList<Entry>) );

            // Fatal Error to be honest but I don't want to crash the operating system
            // due to errors, I want to give it a change to assess errors
            // So It logs into the SystemIO::standardErr
            if (bucket == nullptr)
            {
                SystemIO::perror("[EmbediHashTable::EmbediHashTable()] ERROR: FAILED TO ALLOCATE BUCKET FOR HASHTABLE. IMPLICATIONS IMMINENT");
                break;
            }

            memcpy(bucket, cpy_list, sizeof(EmbediList<Entry>));
            this->buckets[i] = bucket;
        }
    }

    ~EmbediHashTable()
    {
        for (TypeMacros::u32 i = 0; i < this->numBuckets; i++)
        {
            this->buckets[i]->clear();
            basicAllocator.destroy(this->buckets[i]);
        }

        basicAllocator.destroy(this->buckets);
    }

    bool add(const K& key, const V& value)
    {
        if (this->get(key) != nullptr)
            return false;

        TypeMacros::u32 hash = EMBEDI_GENERIC_HASH_FUNCTION<K>(key);
        TypeMacros::u32 index = hash % this->numBuckets;

        EmbediPair<K, V> pair;
        pair.set(key, value);

        if (this->buckets[index]->append(pair))
        {
            this->count++;
            return true;
        }
        else
            return false;
    }

    bool remove(const K& key)
    {
        TypeMacros::u32 hash = EMBEDI_GENERIC_HASH_FUNCTION<K>(key);
        TypeMacros::u32 index = hash % this->numBuckets;

        // start is a DoublyNode<T>* where T = EmbediPair<K, V> ==> DoublyNode<EmbediPair<K, V>>*
        DoublyNode<EmbediPair<K, V>>* start = this->buckets[index].get_head();
        TypeMacros::u32 i = 0;
        while (start->head != nullptr)
        {
            if (start->value.a == key)
                return this->buckets[index].remove(i);
        }

        return false;
    }

    V* get(const K& key)
    {
        TypeMacros::u32 hash = EMBEDI_GENERIC_HASH_FUNCTION<K>(key);
        TypeMacros::u32 index = hash % this->numBuckets;

        // start is a DoublyNode<T>* where T = EmbediPair<K, V> ==> DoublyNode<EmbediPair<K, V>>*
        DoublyNode<EmbediPair<K, V>>* start = this->buckets[index].get_head();
        TypeMacros::u32 i = 0;
        while (start->head != nullptr)
        {
            if (start->value.a == key)
                return &start->value.b;
        }

        return nullptr;
    }

    const Entry* get_entry(const K& key)
    {
        TypeMacros::u32 hash = EMBEDI_GENERIC_HASH_FUNCTION<K>(key);
        TypeMacros::u32 index = hash % this->numBuckets;

        // start is a DoublyNode<T>* where T = EmbediPair<K, V> ==> DoublyNode<EmbediPair<K, V>>*
        DoublyNode<EmbediPair<K, V>>* start = this->buckets[index].get_head();
        TypeMacros::u32 i = 0;
        while (start->head != nullptr)
        {
            if (start->value.a == key)
                return &start->value;
        }

        return nullptr;
    }

    TypeMacros::u32 size() const
    {
        return this->count;
    }

    void clear()
    {
        for (TypeMacros::u32 i = 0; i < this->numBuckets; i++)
            this->buckets[i]->clear();

        this->count = 0;
    }
};

#endif