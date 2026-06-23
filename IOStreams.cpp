#include "IOStreams.hpp"

EmbediFileStream* FileTable::push(EmbediFileStream* fileStream)
{
    if (fileStream == nullptr)
        return nullptr;

    if (this->count + 1 >= MAX_FILE_HANDLES)
        return nullptr;

    this->fileHandles[this->count++] = fileStream;
    return fileStream;
}

EmbediFileStream* FileTable::remove(EmbediFileStream* fdesc)
{
    if (fdesc == nullptr)
        return nullptr;
    
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
{ return Serial.write(_byte) > 0; }

TypeMacros::u32 StandardOutput::write(const TypeMacros::u8* _data, TypeMacros::u32 size)
{ return Serial.write(_data, size); }

void StandardOutput::flush()
{ Serial.flush(); }

TypeMacros::u8 StandardInput::read()
{ return Serial.read(); }

TypeMacros::u32 StandardInput::read(TypeMacros::u8* buffer, TypeMacros::u32 size)
{ return Serial.readBytes(buffer, size); }

TypeMacros::u32 StandardInput::available()
{ return Serial.available(); }

// SDCard File Stream implementation

// This implementation just provides a String view of the original string
// no need to make a new string in memory
inline const char* get_name_from_path(const char* path)
{
    TypeMacros::u32 strsize = strlen(path);

    // We are just gonna assume that i32 never overflows in the negative
    for (TypeMacros::i32 i = strsize - 1; i >= 0; i--)
    {
        if (path[i] == '\\' || path[i] == '/')
            return &path[i + 1];
    }

    return path;
}

SDCardFileStream::SDCardFileStream(const char* path, const TypeMacros::u8 mode)
{
    this->descriptor = {
        get_name_from_path(path),
        path,
        mode
    };
}

SDCardFileStream::~SDCardFileStream()
{
    if (this->file)
        this->file.close();
}

bool SDCardFileStream::open()
{
    if (!this->file)
    {
        int m = 0;

        switch (this->descriptor.mode)
        {
            case READ:
                m = O_RDONLY;
                break;

            case WRITE:
                m = O_WRONLY | O_CREAT;
                break;

            case (READ | WRITE):
                m = O_RDWR;
                break;

            case APPEND:
                m = O_APPEND | O_WRONLY | O_CREAT;
                break;

            case (APPEND | READ):
                m = O_APPEND | O_RDWR | O_CREAT;
                break;

            default:
                return false;
        }

        return this->file.open(this->descriptor.path, m);
    }
    else return true;
}

void SDCardFileStream::close()
{
    if (this->file)
        this->file.close();
}

bool SDCardFileStream::write(const TypeMacros::u8 _byte)
{
    if (this->file && (this->descriptor.mode & WRITE) == WRITE)
    {
        this->file.write(_byte);
        return true;
    }
    else return false;
}

TypeMacros::u32 SDCardFileStream::write(const TypeMacros::u8* _data, TypeMacros::u32 size)
{
    if (this->file && (this->descriptor.mode & WRITE) == WRITE)
    {
        this->file.write(_data, size);
        return size;
    }
    else return 0;
}

void SDCardFileStream::flush()
{
    if ((this->descriptor.mode & WRITE) == WRITE)
        this->file.flush();
}

TypeMacros::u8 SDCardFileStream::read()
{
    if (this->file && (this->descriptor.mode & READ) == READ)
        return this->file.read();

    return 0;
}

TypeMacros::u32 SDCardFileStream::read(TypeMacros::u8* buffer, TypeMacros::u32 size)
{
    if (this->file && (this->descriptor.mode & READ) == READ)
        return this->file.read(buffer, size);

    return 0;
}

void SDCardFileStream::seek(const TypeMacros::u32 pos)
{
    if (this->file)
        this->file.seekSet(pos);
}

TypeMacros::u32 SDCardFileStream::tell()
{
    if (this->file)
        return this->file.curPosition();

    return 0xFFFFFFFF;
}

TypeMacros::u32 SDCardFileStream::available()
{
    if (this->file)
        return this->file.available();

    return 0;
}

// Interfacing functions
namespace SystemIO {
    static EmbediFileStream* standardOut = nullptr;
    static EmbediFileStream* standardInp = nullptr;
    static EmbediFileStream* standardErr = nullptr;

    bool open_file(EmbediFileStream* fileStream)
    {
        if (!fileStream->open())
            return false;

        return allFiles.push(fileStream) != nullptr;
    }

    void close_file(EmbediFileStream* fileStream)
    {
        fileStream->close();
        allFiles.remove(fileStream);
    }

    void close_file(const char* streamName)
    {
        allFiles.remove(allFiles.get_handle(streamName));
    }

    TypeMacros::u8 fgetc(const char* streamName)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        if (stream != nullptr)
        {
            return stream->read();
        }
        else return 0;
    }

    TypeMacros::u8 fgetc(EmbediFileStream* stream)
    {
        if (stream != nullptr)
        {
            return stream->read();
        }
        else return 0;
    }

    void fputc(const char* streamName, const TypeMacros::u8 c)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        if (stream != nullptr)
            stream->write(c);
    }

    void fputc(EmbediFileStream* stream, const TypeMacros::u8 c)
    {
        if (stream != nullptr)
            stream->write(c);
    }

    TypeMacros::u32 fputs(const char* streamName, const char* str)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        if (stream == nullptr)
            return 0;

        const TypeMacros::u32 strsize = strlen(str);
        stream->write(reinterpret_cast<const TypeMacros::u8*>(str), strsize);
        return strsize;
    }

    TypeMacros::u32 fputs(EmbediFileStream* stream, const char* str)
    {
        if (stream == nullptr)
            return 0;

        const TypeMacros::u32 strsize = strlen(str);
        stream->write(reinterpret_cast<const TypeMacros::u8*>(str), strsize);
        return strsize;
    }

    TypeMacros::u32 fgets(const char* streamName, char* buffer, TypeMacros::u32 capSize)
    {
        EmbediFileStream* stream = allFiles.get_handle(streamName);
        if (stream == nullptr)
            return 0;

        TypeMacros::u32 bytesRead = 0;

        while (bytesRead < capSize - 1)
        {
            if (stream->available() > 0)
            {
                TypeMacros::u8 character = stream->read();
                if (character == '\n')
                    break;

                buffer[bytesRead++] = character;
            }
        }

        buffer[bytesRead] = '\0';
        return bytesRead;
    }

    TypeMacros::u32 fgets(EmbediFileStream* stream, char* buffer, TypeMacros::u32 capSize)
    {
        if (stream == nullptr)
            return 0;

        TypeMacros::u32 bytesRead = 0;

        while (bytesRead < capSize - 1)
        {
            if (stream->available() > 0)
            {
                TypeMacros::u8 character = stream->read();
                if (character == '\n')
                    break;

                buffer[bytesRead++] = character;
            }
        }

        buffer[bytesRead] = '\0';
        return bytesRead;
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
                    int carg = va_arg(args, int);
                    stream->write(static_cast<TypeMacros::u8>(carg));
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

    TypeMacros::u32 fprintf(EmbediFileStream* stream, const char* format, ...)
    {
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
                    int carg = va_arg(args, int);
                    stream->write(static_cast<TypeMacros::u8>(carg));
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

    TypeMacros::u8 getchar()
    {
        if (SystemIO::standardInp == nullptr)
            SystemIO::standardInp = allFiles.get_handle(SYSIN_NAME);

        assert(SystemIO::standardInp != nullptr && "[SystemIO::getchar()] FATAL ERROR: Standard Input is not defined");

        return SystemIO::standardInp->read();
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
                    int carg = va_arg(args, int);
                    SystemIO::standardOut->write(static_cast<TypeMacros::u8>(carg));
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

    void printchar(const TypeMacros::u8 c)
    {
        if (SystemIO::standardOut == nullptr)
            SystemIO::standardOut = allFiles.get_handle(SYSOUT_NAME);

        assert(SystemIO::standardOut != nullptr && "[SystemIO::printchar(const char)] FATAL ERROR: Standard Output is not defined");

        SystemIO::standardOut->write(c);
    }

    void perror(const char* str)
    {
        if (SystemIO::standardErr == nullptr)
            SystemIO::standardErr = allFiles.get_handle(SYSERR_NAME);

        assert(SystemIO::standardErr != nullptr && "[SystemIO::perror(const char*)] FATAL ERROR: Standard Error is not defined");

        SystemIO::standardErr->write( reinterpret_cast<const TypeMacros::u8*>(str), strlen(str) );
    }

    void flush()
    {
        if (SystemIO::standardOut == nullptr)
            SystemIO::standardOut = allFiles.get_handle(SYSOUT_NAME);

        assert(SystemIO::standardOut != nullptr && "[SystemIO::printchar(const char)] FATAL ERROR: Standard Output is not defined");

        SystemIO::standardOut->flush();
    }
};