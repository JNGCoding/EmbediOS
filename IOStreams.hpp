#ifndef __EMBEDI_IO_STREAMS_HPP__
#define __EMBEDI_IO_STREAMS_HPP__

#include <Arduino.h>
#include <string.h>
#include "TypeMacros.hpp"

// At max how many files can be opened in the operating system simultaneously
//
// First 3 positions are reserved by stdout, stdin, syserr
constexpr u32 MAX_FILE_HANDLES = 10;

// File Modes
constexpr u8 READ   = 1 << 0;
constexpr u8 WRITE  = 1 << 1;
constexpr u8 APPEND = 1 << 2 | WRITE;

constexpr const char* SYSOUT_NAME = "stdout";
constexpr const char* SYSIN_NAME = "stdin";
constexpr const char* SYSERR_NAME = "stderr";

// This will link against the FileStream
struct FileDescriptor
{
    const char* name;
    const char* path;
    u8 mode;
};

// An interface class for interfacing with file systems
struct EmbediFileStream
{
    FileDescriptor descriptor;
    
    virtual ~EmbediFileStream() = default;
    virtual bool open() { return false; }
    virtual void close() {}
    virtual bool write(const u8 _byte) { return false; }
    virtual u32 write(const u8* _data, u32 size) { return 0; }
    virtual void flush() {}
    virtual u8 read() { return 0; }
    virtual u32 read(u8* buffer, u32 size) { return 0; }
    virtual void seek() {}
    virtual void tell() {}
    virtual u32 available() { return 0; }
};

struct FileTable
{
    EmbediFileStream* fileHandles[ MAX_FILE_HANDLES ];
    u32 count = 0;

    EmbediFileStream* push(EmbediFileStream* fileStream);
    EmbediFileStream* remove(EmbediFileStream* fdesc);
    EmbediFileStream* remove(const int index);
    EmbediFileStream* get_handle(const int index);
    EmbediFileStream* get_handle(const char* streamName);
};

// This is the list that will be handled through-out the filespace
extern FileTable allFiles;

struct StandardOutput : public EmbediFileStream
{
    StandardOutput()
    {
        this->descriptor = {"stdout", "", WRITE};
    }

    bool write(const u8 _byte) override;
    u32 write(const u8* _data, u32 size) override;
    void flush() override;
};

struct StandardInput : public EmbediFileStream
{
    StandardInput()
    {
        this->descriptor = {"stdin", "", READ };
    }
    u8 read() override;
    u32 read(u8* buffer, u32 size) override;
    u32 available() override;
};

struct StandardError : public EmbediFileStream
{
    StandardError()
    {
        this->descriptor = {"stderr", "", READ | WRITE};
    }
};

namespace SystemIO {
    char fgetc(const char* streamName);
    void fputc(const char* streamName, const char c);
    void fputs(const char* streamName, const char* str);
    void fprintf(const char* streamName, const char* format, ...);

    char getchar();
    void printf(const char* format, ...);
    void printchar(const char c);
    void perror(const char* str);
};

#endif