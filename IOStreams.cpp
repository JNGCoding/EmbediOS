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
    for (TypeMacros::u32 i = 0; i < this->count; i++)
    {
        if (this->fileHandles[i] == fdesc)
        {
            for (TypeMacros::u32 j = i; j < this->count - 1; j++)
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
       
    for (TypeMacros::u32 i = index; i < this->count - 1; i++)
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
    for (TypeMacros::u32 i = 0; i < this->count; i++)
    {
        EmbediFileStream* stream = this->fileHandles[i];
        if (strcmp(stream->descriptor.name, streamName) == 0)
            return stream;
    }

    return nullptr;
}

FileTable allFiles;

// Standard IO Stream implementation
bool StandardOutput::write(const TypeMacros::u8 _byte)
{ Serial.write(_byte); return true; }

TypeMacros::u32 StandardOutput::write(const TypeMacros::u8* _data, TypeMacros::u32 size)
{ Serial.write(_data, size); return size; }

void StandardOutput::flush()
{ Serial.flush(); }

TypeMacros::u8 StandardInput::read()
{ return Serial.read(); }

TypeMacros::u32 StandardInput::read(TypeMacros::u8* buffer, TypeMacros::u32 size)
{ return Serial.readBytes(buffer, size); }

TypeMacros::u32 StandardInput::available()
{ return Serial.available(); }

// Interfacing functions
namespace SystemIO {
    static EmbediFileStream* standardOut = nullptr;
    static EmbediFileStream* standardInp = nullptr;
    static EmbediFileStream* standardErr = nullptr;

    char fgetc(const char* streamName)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        if (stream != nullptr)
        {
            return static_cast<char>(stream->read());
        }
        else return 0;
    }

    void fputc(const char* streamName, const char c)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        if (stream != nullptr)
            stream->write(static_cast<const TypeMacros::u8>(c));
    }

    void fputs(const char* streamName, const char* str)
    {
        const TypeMacros::u32 strsize = strlen(str);
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        if (stream != nullptr)
            stream->write(reinterpret_cast<const TypeMacros::u8*>(str), strsize);
    }

    TypeMacros::u32 fprintf(const char* streamName, const char* format, ...)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        if (stream == nullptr)
            return 0;

        TypeMacros::u32 count = 0;
        
        va_list args;
        va_start(args, format);

        // Small buffer to store numbers and stuff
        char smallBuffer[32];

        while (*format != 0)
        {
            if (*format == '%')
            {
                format++;
                if (*format == '%')
                {
                    stream->write('%');
                    count++;
                }
                else if (*format == 's')
                {
                    const char* sarg = va_arg(args, const char*);
                    const TypeMacros::u32 length = strlen(sarg);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(sarg), length);
                    count += length;
                }
                else if (*format == 'd')
                {
                    int iarg = va_arg(args, int);
                    sprintf(smallBuffer, "%d", iarg);
                    const TypeMacros::u32 length = strlen(smallBuffer);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'u')
                {
                    unsigned int uarg = va_arg(args, unsigned int);
                    sprintf(smallBuffer, "%u", uarg);
                    const TypeMacros::u32 length = strlen(smallBuffer);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'f')
                {
                    double farg = va_arg(args, double);
                    sprintf(smallBuffer, "%f", farg);
                    const TypeMacros::u32 length = strlen(smallBuffer);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'c')
                {
                    char carg = va_arg(args, char);
                    stream->write(carg);
                    count++;
                }
                else
                {
                    stream->write(*format);
                    count++;
                }

                format++;
                continue;
            }

            stream->write(*format);

            format++;
            count++;
        }

        va_end(args);

        return count;
    }

    char getchar()
    {
        if (SystemIO::standardInp == nullptr)
            SystemIO::standardInp = allFiles.get_handle(SYSIN_NAME);

        assert(SystemIO::standardInp != nullptr && "[SystemIO::getchar()] FATAL ERROR: Standard Input is not defined");

        return static_cast<char>(SystemIO::standardInp->read());
    }

    TypeMacros::u32 gets(char* buffer, TypeMacros::u32 capSize)
    {
        if (SystemIO::standardInp == nullptr)
            SystemIO::standardInp = allFiles.get_handle(SYSIN_NAME);

        assert(SystemIO::standardInp != nullptr && "[SystemIO::getchar()] FATAL ERROR: Standard Input is not defined");

        TypeMacros::u32 bytesRead = 0;

        while (bytesRead < capSize - 1)
        {
            if (SystemIO::standardInp->available() > 0)
            {
                TypeMacros::u8 character = SystemIO::standardInp->read();
                if (character == '\n')
                    break;

                buffer[bytesRead++] = character;
            }
        }

        buffer[bytesRead] = '\0';
        return bytesRead;
    }

    TypeMacros::u32 printf(const char* format, ...)
    {
        if (SystemIO::standardOut == nullptr)
            SystemIO::standardOut = allFiles.get_handle(SYSOUT_NAME);

        assert(SystemIO::standardOut != nullptr && "[SystemIO::printf(const char*, ...)] FATAL ERROR: Standard Output is not defined");

        TypeMacros::u32 count = 0;
        
        va_list args;
        va_start(args, format);

        // Small buffer to store numbers and stuff
        char smallBuffer[32];

        while (*format != 0)
        {
            if (*format == '%')
            {
                format++;
                if (*format == '%')
                {
                    SystemIO::standardOut->write('%');
                    count++;
                }
                else if (*format == 's')
                {
                    const char* sarg = va_arg(args, const char*);
                    const TypeMacros::u32 length = strlen(sarg);
                    SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(sarg), length);
                    count += length;
                }
                else if (*format == 'd')
                {
                    int iarg = va_arg(args, int);
                    sprintf(smallBuffer, "%d", iarg);
                    const TypeMacros::u32 length = strlen(smallBuffer);
                    SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'u')
                {
                    unsigned int uarg = va_arg(args, unsigned int);
                    sprintf(smallBuffer, "%u", uarg);
                    const TypeMacros::u32 length = strlen(smallBuffer);
                    SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'f')
                {
                    double farg = va_arg(args, double);
                    sprintf(smallBuffer, "%f", farg);
                    const TypeMacros::u32 length = strlen(smallBuffer);
                    SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'c')
                {
                    char carg = va_arg(args, char);
                    SystemIO::standardOut->write(carg);
                    count++;
                }
                else
                {
                    SystemIO::standardOut->write(*format);
                    count++;
                }

                format++;
                continue;
            }

            SystemIO::standardOut->write(*format);

            format++;
            count++;
        }

        va_end(args);

        return count;
    }

    void printchar(const char c)
    {
        if (SystemIO::standardOut == nullptr)
            SystemIO::standardOut = allFiles.get_handle(SYSOUT_NAME);

        assert(SystemIO::standardOut != nullptr && "[SystemIO::printchar(const char)] FATAL ERROR: Standard Output is not defined");

        SystemIO::standardOut->write(static_cast<const TypeMacros::u8>(c));
    }

    void perror(const char* str)
    {
        if (SystemIO::standardErr == nullptr)
            SystemIO::standardErr = allFiles.get_handle(SYSERR_NAME);

        assert(SystemIO::standardErr != nullptr && "[SystemIO::perror(const char*)] FATAL ERROR: Standard Error is not defined");

        SystemIO::standardErr->write( reinterpret_cast<const TypeMacros::u8*>(str), strlen(str) );
    }
};