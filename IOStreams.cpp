#include "IOStreams.hpp"

EmbediFileStream* FileTable::push(EmbediFileStream* fileStream)
{
    if (this->count + 1 >= MAX_FILE_HANDLES)
        return nullptr;

    this->fileHandles[this->count++] = fileStream;
    return fileStream;
}

EmbediFileStream* FileTable::remove(EmbediFileStream* fdesc)
{
    for (u32 i = 0; i < this->count; i++)
    {
        if (this->fileHandles[i] == fdesc)
        {
            for (u32 j = i; j < this->count - 1; j++)
                this->fileHandles[j] = this->fileHandles[j + 1];
        }
    }

    this->count--;
    return fdesc;
}

EmbediFileStream* FileTable::remove(const int index)
{
    if (index >= this->count)
        return nullptr;

    EmbediFileStream* result = this->fileHandles[index];
       
    for (u32 i = index; i < this->count - 1; i++)
        this->fileHandles[i] = this->fileHandles[i + 1];

    this->count--;
    return result;
}

EmbediFileStream* FileTable::get_handle(const int index)
{
    if (index >= this->count)
        return nullptr;

    return this->fileHandles[index];
}

EmbediFileStream* FileTable::get_handle(const char* streamName)
{
    for (u32 i = 0; i < this->count; i++)
    {
        EmbediFileStream* stream = this->fileHandles[i];
        if (strcmp(stream->descriptor.name, streamName) == 0)
            return stream;
    }

    return nullptr;
}

FileTable allFiles;

// Standard IO Stream implementation
bool StandardOutput::write(const u8 _byte)
{ Serial.write(_byte); return true; }

u32 StandardOutput::write(const u8* _data, u32 size)
{ Serial.write(_data, size); return size; }

void StandardOutput::flush()
{ Serial.flush(); }

u8 StandardInput::read()
{ return Serial.read(); }

u32 StandardInput::read(u8* buffer, u32 size)
{ return Serial.readBytes(buffer, size); }

u32 StandardInput::available()
{ return Serial.available(); }

// Interfacing functions
namespace SystemIO {
    static EmbediFileStream* standardOut = nullptr;
    static EmbediFileStream* standardInp = nullptr;
    static EmbediFileStream* standardErr = nullptr;

    char fgetc(const char* streamName)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        return static_cast<char>(stream->read());
    }

    void fputc(const char* streamName, const char c)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        stream->write(static_cast<u8>(c));
    }

    void fputs(const char* streamName, const char* str)
    {
        u32 strsize = strlen(str);
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        stream->write(reinterpret_cast<const u8*>(str), strsize);
    }

    void fprintf(const char* streamName, const char* format, ...)
    {
    }

    char getchar()
    {
        if ( SystemIO::standardInp == nullptr )
            SystemIO::standardInp = allFiles.get_handle(SYSIN_NAME);

        return static_cast<char>(SystemIO::standardInp->read());
    }

    void printf(const char* format, ...)
    {
        if ( SystemIO::standardOut == nullptr )
            SystemIO::standardOut = allFiles.get_handle(SYSOUT_NAME);
    }

    void printchar(const char c)
    {
        if ( SystemIO::standardOut == nullptr )
            SystemIO::standardOut = allFiles.get_handle(SYSOUT_NAME);

        SystemIO::standardOut->write(static_cast<u8>(c));
    }

    void perror(const char* str)
    {
        if ( SystemIO::standardErr == nullptr )
            SystemIO::standardErr = allFiles.get_handle(SYSERR_NAME);
    }
};