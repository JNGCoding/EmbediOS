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

    u32 size()
    { return this->count; }

    ListNode* get_tail()
    { return this->tail; }

    ListNode* get_head()
    { return this->head; }
};

// Generic Static Stack data structure
template<typename T>
class Stack
{
private:
    T* buffer;
    
};

#endif