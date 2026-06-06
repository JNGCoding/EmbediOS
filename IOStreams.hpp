#ifndef __EMBEDI_IO_STREAMS_HPP__
#define __EMBEDI_IO_STREAMS_HPP__

#include <Arduino.h>
#include "TypeMacros.hpp"

// At max how many files can be opened in the operating system simultaneously
//
// First 3 positions are reserved by sysout, sysin, syserr
constexpr u32 MAX_FILE_HANDLES = 10;

// File Modes
constexpr u8 READ   = 1 << 0;
constexpr u8 WRITE  = 1 << 1;
constexpr u8 APPEND = 1 << 2 | WRITE;

// An interface class for interfacing with file systems
struct FileStream
{
    virtual void open() = 0;
    virtual void close() = 0;

    virtual void write(i8 _byte) = 0;
    virtual void write(i8* _data, u32 size) = 0;
    virtual void flush();

    virtual i8 read() = 0;
    virtual void read(const char* buffer, u32 size) = 0;

    virtual void seek() = 0;
    virtual void tell() = 0;

    virtual void available() = 0;
};

// This will link against the FileStream
struct FileDescriptor
{
    int descriptor         = -1;
    int index              = -1;
    u8 mode                = READ;
    FileStream* fileStream = nullptr;
};

struct FileDescriptorList
{
    FileDescriptor fileHandles[ MAX_FILE_HANDLES ];
    u32 count = 0;
    u32 descAuto = 0;

    FileDescriptor* push(FileStream* _fileStream, u8 _mode);
    void remove(FileDescriptor* _fdesc);
    void remove(int _desc);
    FileDescriptor* get_handle(int _desc);
};

char embedi_fgetc(FileDescriptor* file);
void embedi_fputc(FileDescriptor* file, const char c);
void embedi_fputs(FileDescriptor* file, const char* str);
void embedi_fprintf(FileDescriptor* file, const char* format, ...);

char embedi_getchar();
void embedi_printf(const char* format, ...);
void embedi_printchar(const char c);
void embedi_perror(const char* str);

#endif