#ifndef __EMBEDI_DATA_STRUCTURES_HPP__
#define __EMBEDI_DATA_STRUCTURES_HPP__

#include <Arduino.h>
#include <string.h>

#include "TypeMacros.hpp"
#include "../Allocator/Allocator.hpp"

// A generic pair class
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

// A generic singly-linked node
template<typename T>
struct SinglyNode
{
    T value;
    SinglyNode<T>* root = nullptr;
};

// A generic doubly-linked node
template<typename T>
struct DoublyNode
{
    DoublyNode<T>* root = nullptr;
    DoublyNode<T>* head = nullptr;
    T value;
};

// Simple Vector-style Dynamically allocated String type
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
    char get_char(const TypeMacros::u32 index);

    // Returns a bool indicating the presence of a substr in the data buffer
    // `substr` the substring sequence to search for
    bool contains(const char* substr);

    // Returns a bool indicating the presence of a substr in the data buffer
    // `substr` the EmbediString instance whose contents to search for
    bool contains(const EmbediString& substr);

    // Returns true if the data buffer only contains white-space characters
    bool is_blank();

    // Returns true if the data buffer only contains digits
    bool is_digit();

    // Returns true if the data buffer only contains ascii characters
    bool is_ascii();

    // Return true if length = 0
    bool is_nothing();

    // Enforces all the upper characters to their lower counterparts
    void lower();

    // Enforces all the lower characters to their upper counterparts
    void upper();

    // Appends a string to the data buffer
    // `str` string to append
    // returns true if success else false
    bool append(const char* str);

    // Appends a string to the data buffer
    // `str` string to append
    // `start` from which index of the `str` to start
    // returns true if success else false
    bool append(const char* str, const TypeMacros::u32 start);

    // Appends a string to the data buffer
    // `str` string to append
    // `start` from where to start appending in the `str`
    // `end` from where to end appending in the `str`
    // returns true if success else false
    bool append(const char* str, const TypeMacros::u32 start, const TypeMacros::u32 end);

    // Appends a string to the data buffer
    // `c` the character that will be appended
    // returns true if success else false
    bool append(const char c);

    // Appends a string to the data buffer
    // `str` EmbediString instance to append
    // returns true if success else false
    bool append(const EmbediString& str);

    // Appends a string to the data buffer
    // `str` EmbediString instance to append
    // `start` from which index of the `str` to start
    // returns true if success else false
    bool append(const EmbediString& str, const TypeMacros::u32 start);

    // Appends a string to the data buffer
    // `str` EmbediString instance to append
    // `start` from where to start appending in the `str`
    // `end` from where to end appending in the `str`
    // returns true if success else false
    bool append(const EmbediString& str, const TypeMacros::u32 start, const TypeMacros::u32 end);

    // Replaces any previously existing character with a new character in the buffer
    // `oldc` old character (to be replaced)
    // `newc` new character (the replacement)
    void replace(const char oldc, const char newc);

    // Replaces any previously existing character under a substring of the main data buffer
    // `oldc` old character (to be replaced)
    // `newc` new character (the replacement)
    // `start` index where the scanning will start from
    // `end` index where the scanning will be end
    void replace(const char oldc, const char newc, const TypeMacros::u32 start, const TypeMacros::u32 end);

    // Removes all instances of an existing character in the data buffer
    // `oldc` old character (to be removed)
    void remove(const char oldc);

    // Removes all instances of an existing character in the data buffer under a substring of the main data buffer
    // `oldc` old character (to be removed)
    // `start` index where the scanning will start from
    // `end` index where the scanning will be end
    void remove(const char oldc, const TypeMacros::u32 start, const TypeMacros::u32 end);

    // Safely alters the value of the length attribute of the instance
    // `length` the alteration
    void set_length(const TypeMacros::u32 length);

    // Replace the internal string buffer with the given string
    void set(const char* str);    

    // Sets the append index to 0
    // All the previously written data is preserved
    void clear();

    // Shrinks the internal buffer size to the length of the string
    void shrink_to_size();

    // Returns a C-Style string
    const char* c_str();

    // Returns the length of the string
    TypeMacros::u32 length() const
    { return this->len; }

    // Returns true if both strings are equal else false
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
    // Initialize the list instance with an allocator specified defaulted to basicAllocator
    // `_allocator` allocator instance on which the memory for the list will be allocated
    explicit EmbediList(EmbediAllocator* _allocator = &basicAllocator)
    {
        assert(_allocator != nullptr && "[EmbediList::EmbediList()] FATAL ERROR: ALLOCATOR WAS NULL");
        this->allocator = _allocator;
    }

    // Copies another list
    // `other` EmbediList instance to copy from
    // Returns the amount of elements the instance was successfully able to copy
    TypeMacros::u32 copy(EmbediList& other, const TypeMacros::u32 cap_size)
    {
        this->clear();

        ListNode* other_cur = other.tail;
        TypeMacros::u32 elements_added = 0;

        while (other_cur != nullptr) {
            if (elements_added < cap_size)
            {
                if (!this->push(other_cur->value))
                    return elements_added;

                elements_added++;
            }

            other_cur = other_cur->head;
        }

        return elements_added;
    }

    ~EmbediList()
    {
        this->clear();
    }

    // Returns the pointer to the object stored at the index specified if success else nullptr
    // `index` the index position of the object
    T* get(const TypeMacros::u32 index) const
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

    // Changes the value of the object stored at the index specified
    // `index` the index position of the object
    // warning: no action taken if any error occurs
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

    // Appends a value to the ending of the list
    // `element` object to append
    // Returns true if successful else false
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

    // Inserts an element at the index specified
    // `element` object to insert
    // `index` the desired index of the object
    // Returns true if success else false
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

    // Removes an element at the index specified
    // `index` index position of the object
    // Returns true if success else false
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

    // Removes all the objects from the list
    void clear()
    {
        while (this->count > 0)
            this->remove(this->count - 1);
    }

    // Checks if this instance equals to the other instance
    // `other` the second EmbediList instance for the scan
    bool equals(EmbediList& other) const
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

    // Returns the size of the list
    TypeMacros::u32 size() const
    {
        return this->count;
    }

    // Returns a pointer to the DoublyNode<T> instance of the tail of the list
    ListNode* get_tail() const
    {
        return this->tail;
    }

    // Returns a pointer to the DoublyNode<T> instance of the head of the list
    ListNode* get_head() const
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

public:
    // Initalize the stack instance with a pre-allocated buffer to enforce flexibility
    // `buf` the buffer on which the instance will operate
    // `cap_size` maximum capacity of the stack
    explicit EmbediStack(T* buf, const TypeMacros::u32 cap_size) : capacity(cap_size)
    {
        this->buffer = buf;
    }

    // Changes the internal buffer to `buf` and resets the internal
    // information of the data structure
    void set_buffer(T* buf)
    {
        this->buffer = buf;
        this->count = 0;
    }

    // Copies another stack
    // `other` the other EmbediStack instance
    // `cpy_size` Specifies how much data to copy
    // Returns the amount of elements the instance was successfully able to copy
    TypeMacros::u32 copy(EmbediStack& other, const TypeMacros::u32 cpy_size) const
    {
        if (this->buffer == nullptr)
            return 0;

        this->clear();

        for (TypeMacros::u32 i = 0; i < cpy_size && i < other.count && i < this->count; i++)
        {
            if (!this->push(other.buffer[i]))
                return i - 1;
        }

        return cpy_size;
    }

    // Pushes an object onto the stack buffer
    // Returns true if no Stack Overflow error occured else false
    bool push(const T& element)
    {
        if (this->buffer == nullptr)
            return false;
    
        if (this->count >= this->capacity)
            return false;

        this->buffer[this->count++] = element;
        return true;
    }

    // Returns the pointer of the object stored at the head of the stack and decrements the stack pointer, if size = 0 then returns nullptr
    T* pop()
    {
        if (this->buffer == nullptr)
            return nullptr;

        if (this->count <= 0)
            return nullptr;

        return &this->buffer[(this->count--) - 1];
    }

    // Returns the pointer of the object stored at the head of the stack, if size = 0 then returns nullptr
    T* peek()
    {
        if (this->buffer == nullptr)
            return nullptr;

        if (this->count <= 0)
            return nullptr;

        return &this->buffer[this->count - 1];
    }

    // Sets the stack pointer to 0
    void clear()
    {
        this->count = 0;
    }

    // Returns the size of the stack
    const TypeMacros::u32 size() const
    {
        if (this->buffer == nullptr)
            return 0;

        return this->count;
    }

    // Returns the maximum capacity of the stack
    const TypeMacros::u32 cap_size() const
    {
        return this->capacity;
    }

    // Returns true if stack is empty else false
    const bool is_empty() const
    {
        return this->size() == 0;
    }

    // Returns true if stack is full else false
    const bool is_full() const
    {
        return this->size() >= this->cap_size();
    }

    // Returns a constant pointer to the internal stack buffer
    const T* get_buffer() const
    {
        return this->buffer;
    }

    // Checks if this instance equals to the other instance
    // `other` the second EmbediStack instance for the scan
    bool equals(const EmbediStack& other)
    {
        if (this->buffer == nullptr)
            return false;

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

    // Initialize the dynamic stack instance with an allocator specified
    // `_allocator` allocator instance on which the memory for the stack will be allocated
    explicit EmbediDynamicStack(EmbediAllocator* _allocator = &basicAllocator)
    {
        assert(_allocator != nullptr && "[EmbediDynamicStack::EmbediDynamicStack()] FATAL ERROR: ALLOCATOR WAS NULL");

        this->allocator = _allocator;
        this->buffer = static_cast<T*>(this->allocator->alloc(sizeof(T) * EmbediDynamicStack::PRE_ALLOCATE_DURING_CONSTRUCTION));

        assert(this->buffer != nullptr && "[EmbediDynamicStack::EmbediDynamicStack()] FATAL ERROR: OUT OF MEMORY, BUFFER ALLOCATED IS NULL");

        this->capacity = EmbediDynamicStack::PRE_ALLOCATE_DURING_CONSTRUCTION;
    }

    // Copies another dynamic stack
    // `other` EmbediDynamicStack instance to copy from
    // `cpy_size` the amount of the elements to copy
    // Returns the amount of elements the instance was successfully able to copy
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

    // Pushes an object onto the dynamic stack buffer
    // Returns true if success occured else false
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

    // Returns the pointer of the object stored at the head of the stack and decrements the stack pointer, if size = 0 then returns nullptr
    T* pop()
    {
        if (this->count <= 0)
            return nullptr;

        return &this->buffer[(this->count--) - 1];
    }

    // Returns the pointer of the object stored at the head of the stack, if size = 0 then returns nullptr
    T* peek()
    {
        if (this->count <= 0)
            return nullptr;

        return &this->buffer[this->count - 1];
    }

    // Sets the stack pointer to 0
    void clear()
    {
        this->count = 0;
    }

    // Checks if this instance equals to the other instance
    // `other` the second EmbediStack instance for the scan
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

    // Returns the size of the dynamic stack
    TypeMacros::u32 size() const
    {
        return this->count;
    }

    // Returns the current capacity of the dynamic stack
    TypeMacros::u32 cap_size() const
    {
        return this->capacity;
    }

    // Shrinks the internal buffer size to the size of the dynamic stack
    void shrink_to_size()
    {
        if (this->capacity == this->count)
            return;

        char* space = static_cast<T*>(this->allocator->reshape(this->buffer, this->count * sizeof(T)));
        if (space == nullptr)
            return;

        this->data = space;
    }

    // Returns a constant pointer to the internal buffer
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

public:
    // Initializes the RingBuffer with a pre-allocated buffer specified
    // `buf` pre-allocated buffer to enforce flexibility
    // `cap_size` Size of the operating buffer
    explicit EmbediRingBuffer(T* buf, const TypeMacros::u32 cap_size) : capacity(cap_size)
    {
        this->buffer = buf;
    }

    // Changes the internal buffer to `buf` and resets the internal
    // information of the data structure
    void set_buffer(T* buf)
    {
        this->buffer = buf;
        this->appendPtr = 0;
        this->readPtr = 0;
        this->count = 0;
    }

    // Pushes an element on the buffer of the ring buffer
    void push(const T& element)
    {
        if (this->buffer == nullptr)
            return;

        this->buffer[this->appendPtr] = element;
        this->appendPtr = (this->appendPtr + 1) % this->capacity;

        if (this->count < this->capacity)
            this->count++;
        else
            this->readPtr = (this->readPtr + 1) % this->capacity;
    }

    // Pops out an element on the buffer if size > 0 and returns its pointer, if size <= 0 then returns nullptr
    T* pop()
    {
        if (this->buffer == nullptr)
            return nullptr;

        if (this->count == 0)
            return nullptr;

        T* result = &this->buffer[this->readPtr];
        this->readPtr = (this->readPtr + 1) % this->capacity;
        this->count--;

        return result;
    }

    // Returns the pointer to the object in the buffer at readPtr
    T* peek()
    {
        if (this->buffer == nullptr)
            return nullptr;

        if (this->count == 0)
            return nullptr;

        return &this->buffer[this->readPtr];
    }

    // Resets the internal registers to 0
    void clear()
    {
        this->appendPtr = 0;
        this->readPtr = 0;
        this->count = 0;
    }

    // Returns the size of the ring buffer
    TypeMacros::u32 size() const
    {
        if (this->buffer == nullptr)
            return 0;

        return this->count;
    }

    // Copies another ring buffer
    // `other` EmbediRingBuffer instance to copy from
    // Returns the amount of elements the instance was successfully able to copy
    TypeMacros::u32 copy(const EmbediRingBuffer& other)
    {
        if (this->buffer == nullptr)
            return 0;

        if (this->capacity != other.capacity)
            return 0;

        this->clear();
        for (TypeMacros::u32 i = 0; i < other.count; i++)
        {
            this->push(other.buffer[(other.readPtr + i) % other.capacity]);
        }
        return other.count;
    }
};

// Generic Vector Data Structure
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

    // Initialize the vector instance with an allocator specified defaulted to basicAllocator
    // `_allocator` allocator instance on which the memory for the vector buffers will be allocated
    EmbediVector(EmbediAllocator* _allocator = &basicAllocator)
    {
        assert(_allocator != nullptr && "[EmbediVector::EmbediVector()] FATAL ERROR: ALLOCATOR WAS NULL");

        this->allocator = _allocator;
        this->buffer = static_cast<T*>(this->allocator->alloc(sizeof(T) * EmbediVector::PRE_ALLOCATE_DURING_CONSTRUCTION));

        assert(this->buffer != nullptr && "[EmbediVector::EmbediVector()] ERROR: FAILED TO ALLOCATE MEMORY FROM ALLOCATOR");

        this->capacity = EmbediVector::PRE_ALLOCATE_DURING_CONSTRUCTION;
    }

    ~EmbediVector()
    {
        if (this->buffer != nullptr)
            this->allocator->destroy(this->buffer);
    }

    // Copies another vector
    // `other` EmbediVector instance to copy from
    // `cpy_size` the amount of the elements to copy
    // Returns the amount of elements the instance was successfully able to copy
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

    // Appends an element to the end of the vector
    // `value` the object to append
    // Returns true if success else false
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

    // Adds an element at the index specified in the vector
    // `value` the object to append
    // `index` the desired index position of the appending object
    // Returns true if success else false
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

    // Removes the object at the index position specified
    // `index` the index position of the object to be removed
    // Returns true if success else false
    bool remove(const TypeMacros::u32 index)
    {
        if (index >= this->count)
            return false;

        for (TypeMacros::u32 i = index; i < this->count - 1; i++)
            this->buffer[i] = this->buffer[i + 1];
        
        this->count--;

        return true;
    }

    // Pops an object of from the internal buffers and return its pointer, if size = 0 then returns nullptr
    T* pop()
    {
        if (this->count <= 0)
            return nullptr;

        T* result = &this->buffer[this->count - 1];
        this->remove(this->count - 1);
        return result;
    }

    // Returns the pointer to the object at the index specified if success else nullptr
    // `index` index position of the object whose pointer will be returned
    T* get(const TypeMacros::u32 index)
    {
        if (index >= this->count)
            return nullptr;

        return &this->buffer[index];
    }

    // Places a value at the index specified
    // `index` desired index position
    // Returns true if index < size else false
    bool set(const T& value, const TypeMacros::u32 index)
    {
        if (index >= this->count)
            return false;

        this->buffer[index] = value;

        return true;
    }

    // Sets the append index to 0
    void clear()
    {
        this->count = 0;
    }

    // Shrinks the internal buffer size to the size of the vector
    void shrink_to_size()
    {
        if (this->count == this->capacity)
            return;

        T* space = static_cast<T*>(this->allocator->reshape(this->buffer, this->count * sizeof(T)));
        if (space == nullptr)
            return;

        this->buffer = space;
        this->capacity = this->count;
    }

    // Returns the internal buffer as a const pointer
    const T* get_buffer() const
    {
        return this->buffer;
    }

    // Returns the size of the vector
    TypeMacros::u32 size() const
    {
        return this->count;
    }

    // Returns the current capacity of the vector
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

template<typename K>
struct DefaultComparator
{
    static bool equals(const K& v1, const K& v2)
    { return v1 == v2; }
};

template<typename K, typename V, typename Compare = DefaultComparator<K>>
class EmbediHashTable
{
private:
    using Entry = EmbediPair<K, V>;
    EmbediList<Entry>** buckets = nullptr;
    const TypeMacros::u32 numBuckets;
    TypeMacros::u32 count = 0;
    EmbediAllocator* allocator = nullptr;

public:
    constexpr static TypeMacros::u32 DEFAULT_NUM_BUCKETS = 10;

    // Initialize the hash table instance with the number of buckets and allocator (defaulted to basicAllocator) specified
    // `_numBuckets` the number of buckets
    // `_allocator` allocator instance on which the memory of the hash table will be allocated
    explicit EmbediHashTable(const TypeMacros::u32 _numBuckets = EmbediHashTable::DEFAULT_NUM_BUCKETS, EmbediAllocator* _allocator = &basicAllocator)
        : numBuckets(_numBuckets)
    {
        assert(_allocator != nullptr && "[EmbediHashTable::EmbediHashTable()] FATAL ERROR: ALLOCATOR WAS NULL");

        this->allocator = _allocator;

        this->buckets = static_cast<EmbediList<Entry>**>(this->allocator->alloc(sizeof(EmbediList<Entry>*) * _numBuckets));
        assert(this->buckets != nullptr && "[EmbediHashTable::EmbediHashTable()] ERROR: FAILED TO ALLOCATE BUCKETS FOR HASHTABLE");

        // A default list with non - trivial components or whatever for copying
        EmbediList<Entry> cpy_list(_allocator);

        for (TypeMacros::u32 i = 0; i < this->numBuckets; i++)
        {
            EmbediList<Entry>* bucket = static_cast<EmbediList<Entry>*>(this->allocator->alloc(sizeof(EmbediList<Entry>)));

            assert(bucket != nullptr && "[EmbediHashTable::EmbediHashTable()] ERROR: FAILED TO ALLOCATE BUCKET FOR HASHTABLE");

            memcpy(bucket, &cpy_list, sizeof(EmbediList<Entry>));
            this->buckets[i] = bucket;
        }
    }

    ~EmbediHashTable()
    {
        for (TypeMacros::u32 i = 0; i < this->numBuckets; i++)
        {
            this->buckets[i]->clear();
            this->allocator->destroy(this->buckets[i]);
        }

        this->allocator->destroy(this->buckets);
    }

    // Adds a <Key, Value> pair to the internal buckets
    // `key` key of the pair
    // `value` value of the pair
    // Returns true if success else false
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

    // Removes a <key, value> pair from the internal buckets
    // `key` key of the pair
    // Returns true if success else false
    bool remove(const K& key)
    {
        TypeMacros::u32 hash = EMBEDI_GENERIC_HASH_FUNCTION<K>(key);
        TypeMacros::u32 index = hash % this->numBuckets;

        // start is a DoublyNode<T>* where T = EmbediPair<K, V> ==> DoublyNode<EmbediPair<K, V>>*
        DoublyNode<EmbediPair<K, V>>* start = this->buckets[index]->get_tail();
        TypeMacros::u32 i = 0;
        while (start != nullptr)
        {
            if (Compare::equals(start->value.a, key))
            {
                this->buckets[index]->remove(i);
                return true;
            }

            start = start->head;
            i++;
        }

        return false;
    }

    // Returns a pointer to the value under the <key, value> pair if key is found else nullptr
    // `key` key of the pair
    V* get(const K& key)
    {
        TypeMacros::u32 hash = EMBEDI_GENERIC_HASH_FUNCTION<K>(key);
        TypeMacros::u32 index = hash % this->numBuckets;

        // start is a DoublyNode<T>* where T = EmbediPair<K, V> ==> DoublyNode<EmbediPair<K, V>>*
        DoublyNode<EmbediPair<K, V>>* start = this->buckets[index]->get_tail();
        TypeMacros::u32 i = 0;
        while (start != nullptr)
        {
            if (Compare::equals(start->value.a, key))
                return &start->value.b;

            start = start->head;
        }

        return nullptr;
    }

    // Returns a pointer to the <key, value> pair if key is found else nullptr
    // `key` key of the pair
    const Entry* get_entry(const K& key)
    {
        TypeMacros::u32 hash = EMBEDI_GENERIC_HASH_FUNCTION<K>(key);
        TypeMacros::u32 index = hash % this->numBuckets;

        // start is a DoublyNode<T>* where T = EmbediPair<K, V> ==> DoublyNode<EmbediPair<K, V>>*
        DoublyNode<EmbediPair<K, V>>* start = this->buckets[index]->get_tail();
        TypeMacros::u32 i = 0;
        while (start != nullptr)
        {
            if (Compare::equals(start->value.a, key))
                return &start->value;

            start = start->head;
        }

        return nullptr;
    }

    // Returns the amount of elements in the hash table
    TypeMacros::u32 size() const
    {
        return this->count;
    }

    // Removes all the element from all the buckets in the hash table
    void clear()
    {
        for (TypeMacros::u32 i = 0; i < this->numBuckets; i++)
            this->buckets[i]->clear();

        this->count = 0;
    }

    // Returns a const pointer to the bucket in the hashtable, if failed return nullptr
    // This function is strictly their for the internal workings of the library, not to be used by the user in any case
    const EmbediList<Entry>* get_bucket(const TypeMacros::u32 index) const
    {
        if (index >= this->numBuckets)
            return nullptr;

        return this->buckets[index];
    }
};

template<typename T, unsigned int CAPACITY>
class EmbediArray
{
private:
    T buffer[CAPACITY];

public:
    EmbediArray()
    {}

    // Get the element placed at `idx`
    // Return nullptr if `idx` is invalid
    const T* get(const TypeMacros::u32 idx) const
    {
        if (idx >= CAPACITY)
            return nullptr;

        return &this->buffer[idx];
    }

    // Returns a bool if the operation was successfull
    // Places an element inside the array at the designated index (`idx`)
    bool place(const TypeMacros::u32 idx, const T& element)
    {
        if (idx >= CAPACITY)
            return false;

        this->buffer[idx] = element;
        return true;
    }

    // Shifts all the elements from the `index` `shft` places to the left
    // `clear_drift` operation takes a default or clear object which will replace the memory occupied by the drift
    void shift_left(const TypeMacros::u32 index, const TypeMacros::u32 shft = 1, const T* clear_drift = nullptr)
    {
        for (TypeMacros::u32 i = 0; i < shft; i++)
        {
            for (TypeMacros::u32 j = index; j < CAPACITY - 1; j++)
            {
                this->buffer[j] = this->buffer[j + 1];

                if (clear_drift != nullptr)
                    this->buffer[j - 1] = *clear_drift;
            }
        }
    }

    // Shifts all the elements from the `index` `shft` places to the right
    // `clear_drift` operation takes a default or clear object which will replace the memory occupied by the drift 
    void shift_right(const TypeMacros::u32 index, const TypeMacros::u32 shft = 1, const T* clear_drift = nullptr)
    {
        for (TypeMacros::u32 i = 0; i < shft; i++)
        {
            for (TypeMacros::u32 j = CAPACITY; j > index; j--)
            {
                this->buffer[j] = this->buffer[j - 1];
                
                if (clear_drift != nullptr)
                    this->buffer[j - 1] = *clear_drift;
            }
        }
    }

    // Zeroes out the buffer's memory
    // warning: might break the object's residing within completely
    void zero_out()
    {
        memset(this->buffer, 0, CAPACITY * sizeof(T));
    }

    void objset(T obj)
    {
        memset(this->buffer, obj, CAPACITY * sizeof(T));
    }

    // Get the underlying raw C buffer
    const T* get_buf() const
    {
        return this->buffer;
    }
};

// A basic implementation of single-ended FIFO queue
template<typename T>
class EmbediQueue
{
private:
    T* buf = nullptr;

    const TypeMacros::u32 capacity;
    TypeMacros::u32 count = 0, readptr = 0, writeptr = 0;

public:
    // Initializes the queue with `buffer` and its capacity a.k.a `cap`
    EmbediQueue(T* buffer, const TypeMacros::u32 cap) : capacity(cap)
    {
        this->buf = buffer;
    }

    // Changes the internal buffer to `buf` and resets the internal
    // information of the data structure
    void set_buffer(T* buffer)
    {
        this->buf = buffer;
        this->count = 0;
        this->readptr = 0;
        this->writeptr = 0;
    }

    // Pushes an `element` in the internal buffers
    // Returns true if the operation was successful else false
    bool enqueue(const T& element)
    {
        if (this->buf == nullptr)
            return false;

        if (this->is_full())
            return false;

        this->buf[this->writeptr] = element;
        this->writeptr = (this->writeptr + 1) % this->capacity;
        this->count++;

        return true;
    }

    // Pops a value in the internal buffers and returns it
    // Returns the value by pointer if the operation was successful else nullptr
    T* dequeue()
    {
        if (this->buf == nullptr)
            return nullptr;

        if (this->is_empty())
            return nullptr;

        T* element = &this->buf[this->readptr];
        this->readptr = (this->readptr + 1) % this->capacity;
        this->count--;

        return element;
    }

    // Clears the internal counters
    void clear()
    {
        this->writeptr = 0;
        this->readptr = 0;
        this->count = 0;
    }

    // Returns the size of the queue
    const TypeMacros::u32 size() const
    {
        if (this->buf == nullptr)
            return 0;

        return this->count;
    }

    // Returns the maximum capacity of the queue
    const TypeMacros::u32 cap_size() const
    {
        return this->capacity;
    }

    // Returns true if queue is empty else false
    const bool is_empty() const
    {
        return this->size() == 0;
    }

    // Return true if queue is full else false
    const bool is_full() const
    {
        return this->size() >= this->cap_size();
    }
};

#endif