#ifndef __EMBEDI_DATA_STRUCTURES_HPP__
#define __EMBEDI_DATA_STRUCTURES_HPP__

#include "TypeMacros.hpp"
#include "IOStreams.hpp"
#include "Allocator.hpp"
#include <string.h>
#include <assert.h>

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
    T value;
    DoublyNode<T>* head = nullptr;
};

// Generic Dynamically allocated String type
class EmbediString
{
private:
    // Allocator used for memory management of the string buffer
    Allocator* allocator = nullptr;

    // Current allocated capacity of the buffer
    u32 capacity = 0;

    // Current length of the string (number of characters, excluding null terminator)
    u32 len = 0;

    // Pointer to the character buffer storing string contents
    char* data = nullptr;

public:
    // Factor used to grow buffer capacity when expansion is needed
    constexpr static float MULTIPLY_FACTOR = 2.0f;

    // Construct an empty string with the given allocator (defaults to basicAllocator)
    explicit EmbediString(Allocator* _allocator = &basicAllocator);

    // Construct from a C-style string, copying its contents into managed memory
    explicit EmbediString(const char* _data, Allocator* _allocator = &basicAllocator);

    // Copy constructor: creates a new string by copying another EmbediString
    // If allocator is provided, uses that; otherwise inherits from source string
    explicit EmbediString(const EmbediString& str, Allocator* _allocator = nullptr);

    // Destructor: releases allocated memory
    ~EmbediString();

    // Find the first occurrence of a C-string substring; returns index or -1 if not found
    i64 find(const char* substr);

    // Find the first occurrence of another EmbediString; returns index or -1 if not found
    i64 find(const EmbediString& substr);

    // Extract substring [start, end) into buffer; returns true on success
    bool get_substring(EmbediString& buffer, const u32 start, const u32 end);

    // Get pointer to character at given index; returns nullptr if out of bounds
    char* get_char(const u32 index);

    // Check if the string contains the given C-string substring
    bool contains(const char* substr);

    // Check if the string contains the given EmbediString substring
    bool contains(const EmbediString& substr);

    // Check if the string is empty or only whitespace
    bool is_blank();

    // Check if the string consists only of digits
    bool is_digit();

    // Check if all characters are ASCII
    bool is_ascii();

    // Check if the string length is zero (empty)
    bool is_nothing();

    // Convert all characters to lowercase
    void lower();

    // Convert all characters to uppercase
    void upper();

    // Append a C-string to the current buffer
    bool append(const char* str);

    // Append substring of C-string starting at 'start'
    bool append(const char* str, const u32 start);

    // Append substring [start, end) of C-string
    bool append(const char* str, const u32 start, const u32 end);

    // Append another EmbediString
    bool append(const EmbediString& str);

    // Append substring of another EmbediString starting at 'start'
    bool append(const EmbediString& str, const u32 start);

    // Append substring [start, end) of another EmbediString
    bool append(const EmbediString& str, const u32 start, const u32 end);

    // Clear contents and reset length to zero
    void clear();

    // Gets the c-string representation of the string
    char* c_str() const;

    // Get the length of the string
    u32 length() const;

    // Compare equality with another EmbediString
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
    Allocator* allocator = nullptr;
    u32 count = 0;

public:
    explicit EmbediList(Allocator* _allocator = &basicAllocator)
    {
        this->allocator = _allocator;
    }

    explicit EmbediList(const EmbediList& other, Allocator* _allocator = nullptr)
    {
        this->allocator = (_allocator == nullptr) ? other.allocator : _allocator;

        other.head = other.tail;
        while (other.head->head != nullptr) {
            this->append(other.head->value);
            other.head = other.head->head;
        }
        this->append(other.head->value);
    }

    ~EmbediList()
    {
        this->clear();
    }

    T* get(const u32 index)
    {
        if (index >= this->count)
            return nullptr;

        ListNode* temp_top = this->head;

        if (index >= static_cast<u32>(this->count / 2))
        {
            u32 _index = this->count - index - 1;
            for (u32 i = 0; i < _index; i++)
                this->head = this->head->root;
        }
        else
        {
            this->head = this->tail;
            for (u32 i = 0; i < index; i++)
                this->head = this->head->head;
        }

        T* result = &this->head->value;
        this->head = temp_top;

        return result;
    }

    void set(const T& value, const u32 index)
    {
        if (index >= this->count) return;

        ListNode* temp_top = this->head;

        if (index >= static_cast<u32>(this->count / 2))
        {
            u32 _index = this->count - index - 1;
            for (u32 i = 0; i < _index; i++)
                this->head = this->head->root;
        }
        else
        {
            this->head = this->tail;
            for (u32 i = 0; i < index; i++)
                this->head = this->head->head;
        }

        this->head->value = value;
        this->head = temp_top;
    }

    bool push(const T& element)
    {
        if (this->head == nullptr)
        {
            this->head = this->tail = static_cast<ListNode*>(this->allocator->alloc(sizeof(ListNode)));
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

    bool push(const T& element, const u32 index)
    {
        if (index >= this->count) return false;

        ListNode* n = static_cast<ListNode*>(this->allocator->alloc(sizeof(ListNode)));
        if (n == nullptr)
            return false;

        n->root = nullptr;
        n->value = element;
        n->head = nullptr;

        if (index >= static_cast<u32>(this->count / 2))
        {
            u32 _index = this->count - index - 1;
            for (u32 i = 0; i < _index; i++)
                this->head = this->head->root;
        }
        else
        {
            this->head = this->tail;
            for (u32 i = 0; i < index; i++)
                this->head = this->head->head;
        }

        ListNode* prev_node = this->head->root;
        if (prev_node != nullptr)
        {
            prev_node->head = n;
            n->root = prev_node;
        }

        this->head->root = n;
        n->head = this->head;
        this->count++;

        while (this->head->head != nullptr) this->head = this->head->head;
        return true;
    }

    bool remove(const u32 index)
    {
        if (this->count == 0 || index >= this->count)
            return false;

        if (index == 0 && this->count == 1)
        {
            this->allocator->destroy(this->head);
            this->head = this->tail = nullptr;
            this->count--;
            return true;
        }

        if (index == this->count - 1)
        {
            ListNode* n = this->head;
            if (this->count == 1 || this->head == this->tail)
            {
                this->head = this->tail = nullptr;
            }
            else
            {
                this->head = n->root;
                if (this->head != nullptr)
                    this->head->head = nullptr;
            }

            this->allocator->destroy(n);
            this->count--;
        }

        ListNode* temp_top = this->head;
        if (index >= static_cast<u32>(this->count / 2))
        {
            u32 _index = this->count - index - 1;
            for (u32 i = 0; i < _index; i++)
                this->head = this->head->root;
        }
        else
        {
            this->head = this->tail;
            for (u32 i = 0; i < index; i++)
                this->head = this->head->head;
        }

        ListNode* pn = this->head->root;
        ListNode* dn = this->head;

        if (pn == nullptr)
        {
            this->tail = dn->head;
            this->allocator->destroy(this->head);
            return true;
        }

        pn->head = dn->head;
        if (dn->head = nullptr)
            temp_top = pn;

        this->allocator->destroy(dn);
        this->head = temp_top;
        this->count--;

        if (this->count == 0) this->tail = nullptr;

        return true;
    }

    void clear()
    {
        while (this->count > 0)
            this->remove(this->count - 1);
    }

    // Can't take const reference since, we have to iterate over the list
    // So we have to take control of the head* pointers of the list
    bool equals(EmbediList& other)
    {
        if (this->size() != other.count)
            return false;

        bool flag = true;

        ListNode* temp_top = this->head;
        ListNode* other_temp_top = other.head;

        this->head = this->tail;
        other.head = other.tail;

        for (u32 i = 0; i < this->size(); i++)
        {
            if (this->head->value != other.head->value)
            {
                flag = false;
                break;
            }
        }

        this->head = temp_top;
        other.head = other_temp_top;

        return flag;
    }

    u32 size() const
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

// Generic Static Stack data structure
template<typename T>
class EmbediStack
{
private:
    T* buffer = nullptr;
    u32 count = 0;
    const u32 capacity;
    bool heapBufferAllocated = false;

public:
    EmbediStack(T* buf, const u32 cap_size) : capacity(cap_size)
    {
        if (buf == nullptr)
        {
            this->buffer = static_cast<T*>(basicAllocator.alloc(sizeof(T) * cap_size));
            assert(this->buffer != nullptr && "[Stack::Stack(T*, u32)] FATAL ERROR: FAILED TO ALLOCATE BUFFER ON BASIC ALLOCATOR");
            this->heapBufferAllocated = true;
        }
        else this->buffer = buf;
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

    u32 size() const
    {
        return this->count;
    }

    bool equals(const EmbediStack& other)
    {
        if (this->size() != other.size())
            return false;

        for (u32 i = 0; i < this->count; i++)
        {
            if (this->buffer[i] != other.buffer[i])
                return false;
        }

        return true;
    }
};

// Generic Dynamic Stack Data Structure based on an ArenaAllocator
template<typename T>
class EmbediDynamicStack
{
private:
    using StackNode = DoublyNode<T>;

    StackNode* head = nullptr;
    u8 objectsInBlock = 0;
    const u8 preaoc ;
    u32 count = 0;

    T* objBuffer = nullptr;

    ArenaAllocator allocator;

public:
    explicit EmbediDynamicStack(const u8 preAllocationsObjectCount = 1) : preaoc(preAllocationsObjectCount), allocator(sizeof(StackNode) * preaoc)
    {}

    void copy(EmbediDynamicStack& other)
    {
        this->clear();

        StackNode* other_temp_top = other.top;

        while (other.top->root != nullptr)
            other.top = other.top->root;

        for (u32 i = 0; i < other.count; i++)
        {
            this->push( *other.top->value );
            other.top = other.top->head;
        }

        other.top = other_temp_top;
    }

    ~EmbediDynamicStack()
    {
        this->allocator.arena_free();
        if (this->objBuffer != nullptr)
            basicAllocator.destroy(this->objBuffer);
    }

    bool push(const T& element)
    {
        if (this->top == nullptr)
        {
            if (this->top == nullptr)
            {
                this->top == static_cast<StackNode*>(this->allocator.alloc(sizeof(StackNode)));
                if (this->top == nullptr)
                    return false;

                this->top->root = nullptr;
                this->top->value = element;
                this->top->head = nullptr;

                this->count = 1;
                this->objectsInBlock = 1;

                return true;
            }

            // We are at the absolute top of stack
            if (this->top->head == nullptr)
            {
                StackNode* my_node = static_cast<StackNode*>(this->allocator.alloc(sizeof(StackNode)));
                if (my_node == nullptr)
                    return false;

                my_node->root = this->top;
                my_node->value = element;
                my_node->head = nullptr;

                this->top->head = my_node;
                this->top = my_node;
                this->count++;
            }
            else
            {
                this->top = this->top->head;
                this->top->value = element;
                this->count++;
            }

            if (++this->objectsInBlock > this->preaoc) {
                this->objectsInBlock = 0;
            }

            return true;
        }
    }

    T* pop()
    {
        if (this->count <= 0)
            return nullptr;

        if (this->objBuffer == nullptr)
            this->objBuffer = static_cast<T*>(basicAllocator.alloc(sizeof(T)));

        memcpy(this->objBuffer, &this->top->value, sizeof(T));

        this->top = this->top->root;
        this->count--;

        if (--this->objectsInBlock == 0)
        {
            this->allocator.free_block(false);
            if (this->count <= 0)
            {
                this->objectsInBlock = 0;
            }
            else this->objectsInBlock = this->preaoc;
        }

        return this->objBuffer;
    }

    T* peek()
    {
        if (this->count <= 0)
            return nullptr;
        
        return &this->top->value;
    }

    void clear()
    {
        this->objectsInBlock = 0;
        this->count = 0;
        this->allocator.arena_free();
        this->top = nullptr;
    }

    bool equals(EmbediDynamicStack& other)
    {
        if (this->size() != other.size())
            return false;

        StackNode* temp_top = this->top;
        StackNode* other_temp_top = other.top;

        bool flag = true;
        for (u32 i = 0; i < this->count; i++)
        {
            if (this->top->value != other.top->value)
            {
                flag = false;
                break;
            }

            this->top = this->top->root;
            other.top = other.top->root;
        }

        this->top = temp_top;
        other.top = other_temp_top;

        return flag;
    }

    u32 size() const
    {
        return this->count;
    }
};

template<typename T>
class EmbediRingBuffer
{
private:
    T* buffer = nullptr;
    const u32 capacity;

    u32 appendPtr = 0;
    u32 readPtr = 0;

    bool heapBufferAllocated = false;

public:
    explicit EmbediRingBuffer(T* buf, const u32 cap_size) : capacity(cap_size)
    {
        if (buf == nullptr)
        {
            this->buffer = static_cast<T*>(basicAllocator.alloc(sizeof(T) * cap_size));
            this->heapBufferAllocated = true;
            assert(this->buffer != nullptr && "[EmbediRingBuffer::EmbediRingBuffer(T*, u32)] FATAL ERROR: FAILED TO ALLOCATE BUFFER ON BASIC ALLOCATOR");
        }
        else this->buffer = buf;
    }

    ~EmbediRingBuffer()
    {
        if (this->headBufferAllocated)
            basicAllocator.destroy(this->buffer);
    }

    void append(const T& element)
    {
    }

    T* read()
    {
    }

    u32 size() const
    {
        return readPtr;
    }

    void clear()
    {
        this->appendPtr = 0;
        this->readPtr = 0;
    }
};

#endif