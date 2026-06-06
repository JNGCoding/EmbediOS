#include "IOStreams.hpp"

FileDescriptor* FileDescriptorList::push(FileStream* _fileStream, u8 _mode)
{
    if (this->count + 1 >= MAX_FILE_HANDLES)
        return nullptr;

    this->fileHandles[ this->count ].fileStream = _fileStream;
    this->fileHandles[ this->count ].descriptor = this->descAuto++;
    this->fileHandles[ this->count ].mode       = _mode;

    this->count++;
    return &this->fileHandles[ this->count - 1 ];
}

void FileDescriptorList::remove(FileDescriptor* fdesc)
{
}

void FileDescriptorList::remove(int desc)
{
}

FileDescriptor* FileDescriptorList::get_handle(int desc)
{
    return nullptr;
}

// Interfacing functions
char embedi_fgetc(FileDescriptor* file)
{
    return 'A';
}

void embedi_fputc(FileDescriptor* file, const char c)
{}

void embedi_fputs(FileDescriptor* file, const char* str)
{}

void embedi_fprintf(FileDescriptor* file, const char* format, ...)
{}

char embedi_getchar()
{
    return 'A';
}

void embedi_printf(const char* format, ...)
{}

void embedi_printchar(const char c)
{}

void embedi_perror(const char* str)
{}