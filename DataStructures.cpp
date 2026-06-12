#include <string.h>
#include "DataStructures.hpp"

// --------------------------- EMBEDI STRING ------------------------
constexpr TypeMacros::u32 INTEGER_STRING_MAX_SIZE = 12;
constexpr TypeMacros::u32 DOUBLE_STRING_MAX_SIZE  = 32;

inline char get_small(char c) { return (c >= 65 && c <= 90) ? c + 32 : c; }
inline char get_capital(char c) { return (c >= 97 && c <= 122) ? c - 32 : c; }

EmbediString::EmbediString(EmbediAllocator* _allocator)
{
    assert(_allocator != nullptr && "[EmbediString::EmbediString()] FATAL ERROR: ALLOCATOR WAS NULL");

    this->allocator = _allocator;

    this->data = static_cast<char*>(this->allocator->alloc(1));
    if (this->data == nullptr)
    {
        SystemIO::perror("[EmbediString::EmbediString()] ERROR: FAILED TO ALLOCATE MEMORY FROM ALLOCATOR");
        return;
    }

    this->data[0] = '\0';
    this->capacity = 1;
}

EmbediString::EmbediString(const char* _data, EmbediAllocator* _allocator)
{
    assert(_allocator != nullptr && "[EmbediString::EmbediString(const char*)] FATAL ERROR: ALLOCATOR WAS NULL");

    if (_data == nullptr)
    {
        SystemIO::perror("[EmbediString::EmbediString(const char*)] WARNING: NULLPTR WAS PASSED TO THE ARGUMENTS, CALLING DIFFERENT CONSTRUCTOR");

        // EmbediString::EmbediString() body
        this->allocator = _allocator;

        this->data = static_cast<char*>(this->allocator->alloc(1));
        if (this->data == nullptr)
        {
            SystemIO::perror("[EmbediString::EmbediString()] ERROR: FAILED TO ALLOCATE MEMORY FROM ALLOCATOR");
            return;
        }

        this->data[0] = '\0';
        this->capacity = 1;

        return;
    }

    this->allocator = _allocator;
    const TypeMacros::u32 strsize = strlen(_data);

    this->data = static_cast<char*>(this->allocator->alloc(strsize));
    if (this->data == nullptr)
    {
        SystemIO::perror("[EmbediString::EmbediString(const char*)] ERROR: FAILED TO ALLOCATE MEMORY FROM ALLOCATOR");
        return;
    }

    this->capacity = strsize;
    this->len = strsize;

    memcpy(this->data, _data, this->len * sizeof(char));
}

EmbediString::EmbediString(const EmbediString& str, EmbediAllocator* _allocator)
{
    this->allocator = (_allocator == nullptr) ? str.allocator : _allocator;

    this->data = static_cast<char*>(this->allocator->alloc(str.len));
    if (this->data == nullptr)
    {
        SystemIO::perror("[EmbediString::EmbediString(const char*)] ERROR: FAILED TO ALLOCATE MEMORY FROM ALLOCATOR");
        return;
    }

    // preferred just in case the object is not mutated and is short lived
    this->capacity = str.len;
    this->len = str.len;

    memcpy(this->data, str.data, this->len);
}

EmbediString::~EmbediString()
{
    if (this->data != nullptr)
        this->allocator->destroy(this->data);
}

TypeMacros::i64 EmbediString::find(const char* substr)
{
    if (substr == nullptr)
        return -1;

    if (this->len == 0)
        return -1;

    const TypeMacros::u32 strsize = strlen(substr);

    for (TypeMacros::u32 i = 0; i + strsize < this->len; i++)
    {
        if (this->data[i] == substr[0])
        {
            bool flag = true;
            for (TypeMacros::u32 j = i + 1; j - i < strsize; j++)
            {
                if (this->data[j] != substr[j - i])
                {
                    flag = false;
                    break;
                }
            }

            if (flag)
                return static_cast<TypeMacros::i64>(i);
        }
    }

    return -1;
}

TypeMacros::i64 EmbediString::find(const EmbediString& substr)
{
    if (this->len == 0)
        return -1;

    for (TypeMacros::u32 i = 0; i + substr.len < this->len; i++)
    {
        if (this->data[i] == substr.data[0])
        {
            bool flag = true;
            for (TypeMacros::u32 j = i + 1; j - i < substr.len; j++)
            {
                if (this->data[j] != substr.data[j - i])
                {
                    flag = false;
                    break;
                }
            }

            if (flag)
                return static_cast<TypeMacros::i64>(i);
        }
    }

    return -1;
}

bool EmbediString::get_substring(EmbediString& buffer, const TypeMacros::u32 start, const TypeMacros::u32 end)
{
    buffer.clear();
    return buffer.append(*this, start, end);
}

char* EmbediString::get_char(const TypeMacros::u32 index)
{
    if (index >= this->len)
        return nullptr;

    return &this->data[index];
}

bool EmbediString::contains(const char* substr)
{
    return this->find(substr) != -1;
}

bool EmbediString::contains(const EmbediString& substr)
{
    return this->find(substr) != -1;
}

bool EmbediString::is_blank()
{
    if (this->len == 0) return true;

    for (size_t i = 0; i < this->len; i++) {
        if (
            this->data[i] != ' '  ||
            this->data[i] != '\n' ||
            this->data[i] != '\v' ||
            this->data[i] != '\b' ||
            this->data[i] != '\f' ||
            this->data[i] != '\t'
        ) {
            return false;
        }
    }

    return true;
}

bool EmbediString::is_digit()
{
    for (size_t i = 0; i < this->len; i++)
    {
        if (this->data[i] < '0' || this->data[i] > '9')
            return false;
    }

    return true;
}

bool EmbediString::is_ascii()
{
    for (size_t i = 0; i < this->len; i++) {
        if (static_cast<u8>(this->data[i]) > 127) {
            return false;
        }
    }
    
    return true;
}

bool EmbediString::is_nothing()
{
    return this->len == 0;
}

void EmbediString::lower()
{
    for (TypeMacros::u32 i = 0; i < this->len; i++)
        this->data[i] = get_small(this->data[i]);
}

void EmbediString::upper()
{
    for (TypeMacros::u32 i = 0; i < this->len; i++)
        this->data[i] = get_capital(this->data[i]);
}

bool EmbediString::append(const char* str)
{
    if (str == nullptr) return false;

    const TypeMacros::u32 strsize = strlen(str);
    if (this->len + strsize >= this->capacity)
    {
        TypeMacros::u32 ncap = this->capacity * EmbediString::MULTIPLY_FACTOR;

        void* space = this->allocator->reshape(this->data, ncap);
        if (space == nullptr)
            return false;

        this->data = static_cast<char*>(space);
        this->capacity = ncap;
    }

    memcpy(this->data + this->len, str, strsize);
    this->len += strsize;

    return true;
}

bool EmbediString::append(const char* str, const TypeMacros::u32 start)
{
    if (str == nullptr) return false;

    const TypeMacros::u32 strlength = strlen(str);
    const TypeMacros::u32 strsize = (start >= strlength) ? strlength : strlength - start;

    if (this->len + strsize >= this->capacity)
    {
        TypeMacros::u32 ncap = this->capacity * EmbediString::MULTIPLY_FACTOR;

        void* space = this->allocator->reshape(this->data, ncap);
        if (space == nullptr)
            return false;

        this->data = static_cast<char*>(space);
        this->capacity = ncap;
    }

    memcpy(this->data + this->len, str + start, strsize);
    this->len += strsize;

    return true;
}

bool EmbediString::append(const char* str, const TypeMacros::u32 start, const TypeMacros::u32 end)
{
    if (str == nullptr) return false;

    const TypeMacros::u32 strlength = strlen(str);

    TypeMacros::u32 s, e;
    if (start > end)
    {
        s = end;
        e = start;
    }
    else
    {
        s = start;
        e = end;
    }

    const TypeMacros::u32 strsize = e - s;
    if (strsize > strlength)
        e = strlength;

    if (this->len + strsize >= this->capacity)
    {
        TypeMacros::u32 ncap = this->capacity * EmbediString::MULTIPLY_FACTOR;

        void* space = this->allocator->reshape(this->data, ncap);
        if (space == nullptr)
            return false;

        this->data = static_cast<char*>(space);
        this->capacity = ncap;
    }

    memcpy(this->data + this->len, str + start, strsize);
    this->len += strsize;

    return true;
}

bool EmbediString::append(const EmbediString& str)
{
    const TypeMacros::u32 strsize = str.len;
    if (this->len + strsize >= this->capacity)
    {
        TypeMacros::u32 ncap = this->capacity * EmbediString::MULTIPLY_FACTOR;

        void* space = this->allocator->reshape(this->data, ncap);
        if (space == nullptr)
            return false;

        this->data = static_cast<char*>(space);
        this->capacity = ncap;
    }

    memcpy(this->data + this->len, str.data, strsize);
    this->len += strsize;

    return true;
}

bool EmbediString::append(const EmbediString& str, const TypeMacros::u32 start)
{
    const TypeMacros::u32 strlength = str.len;
    const TypeMacros::u32 strsize = (start >= strlength) ? strlength : strlength - start;

    if (this->len + strsize >= this->capacity)
    {
        TypeMacros::u32 ncap = this->capacity * EmbediString::MULTIPLY_FACTOR;

        void* space = this->allocator->reshape(this->data, ncap);
        if (space == nullptr)
            return false;

        this->data = static_cast<char*>(space);
        this->capacity = ncap;
    }

    memcpy(this->data + this->len, str.data + start, strsize);
    this->len += strsize;

    return true;
}

bool EmbediString::append(const EmbediString& str, const TypeMacros::u32 start, const TypeMacros::u32 end)
{
    const TypeMacros::u32 strlength = str.len;

    TypeMacros::u32 s, e;
    if (start > end)
    {
        s = end;
        e = start;
    }
    else
    {
        s = start;
        e = end;
    }

    const TypeMacros::u32 strsize = e - s;
    if (strsize > strlength)
        e = strlength;

    if (this->len + strsize >= this->capacity)
    {
        TypeMacros::u32 ncap = this->capacity * EmbediString::MULTIPLY_FACTOR;

        void* space = this->allocator->reshape(this->data, ncap);
        if (space == nullptr)
            return false;

        this->data = static_cast<char*>(space);
        this->capacity = ncap;
    }

    memcpy(this->data + this->len, str.data + start, strsize);
    this->len += strsize;

    return true;
}

void EmbediString::clear()
{
    this->len = 0;
}

void EmbediString::shrink_to_size()
{
    if (this->capacity == this->len)
        return;

    char* space = static_cast<char*>(this->allocator->reshape(this->data, this->len * sizeof(char)));
    if (space == nullptr)
        return;

    this->data = space;
}

bool EmbediString::replace(const char* old, const char* sequence)
{
    const TypeMacros::u32 old_len = strlen(old);
    const TypeMacros::u32 new_len = strlen(sequence);

    // We will add it to the current length after each change
    TypeMacros::u32 delta_len = new_len - old_len;

    // We don't have to allocate any new memory
    if (old_len > new_len)
    {
        for (TypeMacros::u32 i = 0; i + new_len < this->len; i++)
        {
            if (this->data[i] == sequence[0])
            {
                bool flag = true;
                for (TypeMacros::u32 j = i + 1; j - i < new_len; j++)
                {
                    if (this->data[j] != sequence[j - i])
                    {
                        flag = false;
                        break;
                    }
                }

                if (flag)
                {
                    // TODO: WE HAVE FOUND A MATCH
                }
            }
        }
    }
    else if (old_len == new_len)
    {
        for (TypeMacros::u32 i = 0; i + new_len < this->len; i++)
        {
            if (this->data[i] == sequence[0])
            {
                bool flag = true;
                for (TypeMacros::u32 j = i + 1; j - i < new_len; j++)
                {
                    if (this->data[j] != sequence[j - i])
                    {
                        flag = false;
                        i = j;
                        break;
                    }
                }

                if (flag)
                {
                    for (TypeMacros::u32 j = i; j - i < new_len; j++)
                        this->data[j] = sequence[j - i];

                    i += new_len;
                }
            }
        }
    }
    // Consider: We may have to allocate new memory for extended string
    else
    {
    }

    return false;
}

const char* EmbediString::c_str()
{
    this->data[this->len] = '\0';
    return this->data;
}

TypeMacros::u32 EmbediString::length() const
{
    return this->len;
}

bool EmbediString::equals(const EmbediString& other)
{
    if (this->len != other.len) return false;
    if (!this->data || !other.data) return false;

    for (TypeMacros::u32 i = 0; i < this->len; i++)
        if (this->data[i] != other.data[i]) return false;

    return true;
}