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

void FileTable::post_all_handles()
{
    for (TypeMacros::u32 i = 0; i < this->count; i++)
        this->fileHandles[i]->post();
}

void FileTable::poll_all_handles()
{
    for (TypeMacros::u32 i = 0; i < this->count; i++)
        this->fileHandles[i]->poll();
}

// SDCard File Stream implementation

// This implementation just provides a String view of the original string
// no need to make a new string in memory
#ifdef COMPILE_WITH_SD_INITIALIZATION

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

    return 0;
}

TypeMacros::u32 SDCardFileStream::available()
{
    if (this->file)
        return this->file.available();

    return 0;
}

#endif

// Interfacing functions
namespace SystemIO {
    static EmbediFileStream* standardOut = nullptr;
    static EmbediFileStream* standardInp = nullptr;
    static EmbediFileStream* standardErr = nullptr;

    bool open_file(EmbediFileStream* fileStream)
    {
        if (!fileStream->open())
            return false;

        return __filetable.push(fileStream) != nullptr;
    }

    void close_file(EmbediFileStream* fileStream)
    {
        fileStream->close();
        __filetable.remove(fileStream);
    }

    void close_file(const char* streamName)
    {
        __filetable.remove(__filetable.get_handle(streamName));
    }

    void fflush(const char* streamName)
    {
        EmbediFileStream* stream = __filetable.get_handle(streamName);
        if (stream != nullptr)
            stream->flush();
    }

    void fflush(EmbediFileStream* stream)
    {
        if (stream != nullptr)
            stream->flush();
    }

    TypeMacros::u32 favailable(const char* streamName)
    {
        EmbediFileStream* stream = __filetable.get_handle(streamName);
        if (stream == nullptr)
            return 0;

        return stream->available();
    }

    TypeMacros::u32 favailable(EmbediFileStream* stream)
    {
        if (stream == nullptr)
            return 0;

        return stream->available();
    }

    TypeMacros::u8 fgetc(const char* streamName)
    {
        EmbediFileStream* stream = __filetable.get_handle(streamName);
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
        EmbediFileStream* stream = __filetable.get_handle(streamName);
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
        EmbediFileStream* stream = __filetable.get_handle(streamName);
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

    TypeMacros::u32 fgets(const char* streamName, char* buffer, TypeMacros::u32 capSize, const TypeMacros::u32 block_threshold)
    {
        EmbediFileStream* stream = __filetable.get_handle(streamName);
        if (stream == nullptr)
            return 0;

        TypeMacros::u32 bytesRead = 0;
        TypeMacros::u32 start = millis();

        while (bytesRead < capSize - 1 && millis() - start <= block_threshold)
        {
            if (stream->available() > 0)
            {
                TypeMacros::u8 character = stream->read();
                if (character == '\n')
                    break;

                buffer[bytesRead++] = character;
                start = millis();
            }
        }

        buffer[bytesRead] = '\0';
        return bytesRead;
    }

    TypeMacros::u32 fgets(EmbediFileStream* stream, char* buffer, TypeMacros::u32 capSize, const TypeMacros::u32 block_threshold)
    {
        if (stream == nullptr)
            return 0;

        TypeMacros::u32 bytesRead = 0;
        TypeMacros::u32 start = millis();

        while (bytesRead < capSize - 1 && millis() - start <= block_threshold)
        {
            if (stream->available() > 0)
            {
                TypeMacros::u8 character = stream->read();
                if (character == '\n')
                    break;

                buffer[bytesRead++] = character;
                start = millis();
            }
        }

        buffer[bytesRead] = '\0';
        return bytesRead;
    }

    TypeMacros::u32 fprintf(const char* streamName, const char* format, ...)
    {
        EmbediFileStream* stream = __filetable.get_handle(streamName);
        if (stream == nullptr)
            return 0;

        TypeMacros::u32 count = 0;
        
        va_list args;
        va_start(args, format);

        // Small buffer to store numbers and stuff
        char smallBuffer[320];

        while (*format != 0)
        {
            if (*format == '%')
            {
                format++;

                if (*format == '\0')
                    break;

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
                else if (*format == 'd' || *format == 'i')
                {
                    int iarg = va_arg(args, int);
                    const TypeMacros::u32 length = sprintf(smallBuffer, "%d", iarg);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'u')
                {
                    unsigned int uarg = va_arg(args, unsigned int);
                    const TypeMacros::u32 length = sprintf(smallBuffer, "%u", uarg);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'f')
                {
                    double farg = va_arg(args, double);
                    const TypeMacros::u32 length = sprintf(smallBuffer, "%f", farg);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'l')
                {
                    char postfix = *(format + 1);
                    if (is_space(postfix))
                    {
                        long larg = va_arg(args, long);
                        const TypeMacros::u32 length = sprintf(smallBuffer, "%ld", larg);
                        stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                        count += length;
                    }
                    else
                    {
                        format++;

                        if (postfix == 'd')
                        {
                            long int larg = va_arg(args, long int);
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%ld", larg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'u')
                        {
                            unsigned long int larg = va_arg(args, unsigned long int);
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%lu", larg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'l')
                        {
                            char postpostfix = *(format + 1);

                            if (is_space(postpostfix))
                            {
                                long long int llarg = va_arg(args, long long int);
                                const TypeMacros::u32 length = sprintf(smallBuffer, "%lld", llarg);
                                stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                                count += length;
                            }
                            else
                            {
                                format++;
                                if (postpostfix == 'u')
                                {
                                    unsigned long long int llarg = va_arg(args, unsigned long long int);
                                    const TypeMacros::u32 length = sprintf(smallBuffer, "%llu", llarg);
                                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                                    count += length;
                                }
                                else if (postpostfix == 'd')
                                {
                                    long long int llarg = va_arg(args, long long int);
                                    const TypeMacros::u32 length = sprintf(smallBuffer, "%lld", llarg);
                                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                                    count += length;
                                }
                            }
                        }
                    }
                }
                else if (*format == 'h')
                {
                    char postfix = *(format + 1);
                    if (is_space(postfix))
                    {
                        short sharg = static_cast<short>(va_arg(args, int));
                        const TypeMacros::u32 length = sprintf(smallBuffer, "%hd", sharg);
                        stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                        count += length;
                    }
                    else
                    {
                        format++;

                        if (postfix == 'd')
                        {
                            short int sharg = static_cast<short int>(va_arg(args, int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hd", sharg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'u')
                        {
                            unsigned short int sharg = static_cast<unsigned short int>(va_arg(args, int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hu", sharg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                    }
                }
                else if (*format == 'b')
                {
                    char postfix = *(format + 1);
                    if (is_space(postfix))
                    {
                        char barg = static_cast<char>(va_arg(args, int));
                        const TypeMacros::u32 length = sprintf(smallBuffer, "%hhd", barg);
                        stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                        count += length;
                    }
                    else
                    {
                        format++;
                        if (postfix == 'd')
                        {
                            char barg = static_cast<char>(va_arg(args, int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hhd", barg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'u')
                        {
                            unsigned char barg = static_cast<unsigned char>(va_arg(args, unsigned int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hhu", barg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                    }
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
        char smallBuffer[320];

        while (*format != 0)
        {
            if (*format == '%')
            {
                format++;

                if (*format == '\0')
                    break;

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
                else if (*format == 'd' || *format == 'i')
                {
                    int iarg = va_arg(args, int);
                    const TypeMacros::u32 length = sprintf(smallBuffer, "%d", iarg);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'u')
                {
                    unsigned int uarg = va_arg(args, unsigned int);
                    const TypeMacros::u32 length = sprintf(smallBuffer, "%u", uarg);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'f')
                {
                    double farg = va_arg(args, double);
                    const TypeMacros::u32 length = sprintf(smallBuffer, "%f", farg);
                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'l')
                {
                    char postfix = *(format + 1);
                    if (is_space(postfix))
                    {
                        long larg = va_arg(args, long);
                        const TypeMacros::u32 length = sprintf(smallBuffer, "%ld", larg);
                        stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                        count += length;
                    }
                    else
                    {
                        format++;

                        if (postfix == 'd')
                        {
                            long int larg = va_arg(args, long int);
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%ld", larg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'u')
                        {
                            unsigned long int larg = va_arg(args, unsigned long int);
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%lu", larg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'l')
                        {
                            char postpostfix = *(format + 1);

                            if (is_space(postpostfix))
                            {
                                long long int llarg = va_arg(args, long long int);
                                const TypeMacros::u32 length = sprintf(smallBuffer, "%lld", llarg);
                                stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                                count += length;
                            }
                            else
                            {
                                format++;
                                if (postpostfix == 'u')
                                {
                                    unsigned long long int llarg = va_arg(args, unsigned long long int);
                                    const TypeMacros::u32 length = sprintf(smallBuffer, "%llu", llarg);
                                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                                    count += length;
                                }
                                else if (postpostfix == 'd')
                                {
                                    long long int llarg = va_arg(args, long long int);
                                    const TypeMacros::u32 length = sprintf(smallBuffer, "%lld", llarg);
                                    stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                                    count += length;
                                }
                            }
                        }
                    }
                }
                else if (*format == 'h')
                {
                    char postfix = *(format + 1);
                    if (is_space(postfix))
                    {
                        short sharg = static_cast<short>(va_arg(args, int));
                        const TypeMacros::u32 length = sprintf(smallBuffer, "%hd", sharg);
                        stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                        count += length;
                    }
                    else
                    {
                        format++;

                        if (postfix == 'd')
                        {
                            short int sharg = static_cast<short int>(va_arg(args, int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hd", sharg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'u')
                        {
                            unsigned short int sharg = static_cast<unsigned short int>(va_arg(args, int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hu", sharg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                    }
                }
                else if (*format == 'b')
                {
                    char postfix = *(format + 1);
                    if (is_space(postfix))
                    {
                        char barg = static_cast<char>(va_arg(args, int));
                        const TypeMacros::u32 length = sprintf(smallBuffer, "%hhd", barg);
                        stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                        count += length;
                    }
                    else
                    {
                        format++;
                        if (postfix == 'd')
                        {
                            char barg = static_cast<char>(va_arg(args, int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hhd", barg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'u')
                        {
                            unsigned char barg = static_cast<unsigned char>(va_arg(args, unsigned int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hhu", barg);
                            stream->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                    }
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
            SystemIO::standardInp = __filetable.get_handle(SYSIN_NAME);

        assert(SystemIO::standardInp != nullptr && "[SystemIO::getchar()] FATAL ERROR: Standard Input is not defined");

        return SystemIO::standardInp->read();
    }

    TypeMacros::u32 gets(char* buffer, TypeMacros::u32 capSize, const TypeMacros::u32 block_threshold)
    {
        if (SystemIO::standardInp == nullptr)
            SystemIO::standardInp = __filetable.get_handle(SYSIN_NAME);

        assert(SystemIO::standardInp != nullptr && "[SystemIO::getchar()] FATAL ERROR: Standard Input is not defined");

        TypeMacros::u32 bytesRead = 0;
        TypeMacros::u32 start = millis();

        while (bytesRead < capSize - 1 && millis() - start <= block_threshold)
        {
            if (SystemIO::standardInp->available() > 0)
            {
                TypeMacros::u8 character = SystemIO::standardInp->read();
                if (character == '\n')
                    break;

                buffer[bytesRead++] = character;
                start = millis();
            }
        }

        buffer[bytesRead] = '\0';
        return bytesRead;
    }

    TypeMacros::u32 printf(const char* format, ...)
    {
        if (SystemIO::standardOut == nullptr)
            SystemIO::standardOut = __filetable.get_handle(SYSOUT_NAME);

        assert(SystemIO::standardOut != nullptr && "[SystemIO::printf(const char*, ...)] FATAL ERROR: Standard Output is not defined");

        TypeMacros::u32 count = 0;
        
        va_list args;
        va_start(args, format);

        // Small buffer to store numbers and stuff
        char smallBuffer[320];

        while (*format != 0)
        {
            if (*format == '%')
            {
                format++;

                if (*format == '\0')
                    break;

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
                else if (*format == 'd' || *format == 'i')
                {
                    int iarg = va_arg(args, int);
                    const TypeMacros::u32 length = sprintf(smallBuffer, "%d", iarg);
                    SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'u')
                {
                    unsigned int uarg = va_arg(args, unsigned int);
                    const TypeMacros::u32 length = sprintf(smallBuffer, "%u", uarg);
                    SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'f')
                {
                    double farg = va_arg(args, double);
                    const TypeMacros::u32 length = sprintf(smallBuffer, "%f", farg);
                    SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                    count += length;
                }
                else if (*format == 'l')
                {
                    char postfix = *(format + 1);
                    if (is_space(postfix))
                    {
                        long larg = va_arg(args, long);
                        const TypeMacros::u32 length = sprintf(smallBuffer, "%ld", larg);
                        SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                        count += length;
                    }
                    else
                    {
                        format++;

                        if (postfix == 'd')
                        {
                            long int larg = va_arg(args, long int);
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%ld", larg);
                            SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'u')
                        {
                            unsigned long int larg = va_arg(args, unsigned long int);
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%lu", larg);
                            SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'l')
                        {
                            char postpostfix = *(format + 1);

                            if (is_space(postpostfix))
                            {
                                long long int llarg = va_arg(args, long long int);
                                const TypeMacros::u32 length = sprintf(smallBuffer, "%lld", llarg);
                                SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                                count += length;
                            }
                            else
                            {
                                format++;
                                if (postpostfix == 'u')
                                {
                                    unsigned long long int llarg = va_arg(args, unsigned long long int);
                                    const TypeMacros::u32 length = sprintf(smallBuffer, "%llu", llarg);
                                    SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                                    count += length;
                                }
                                else if (postpostfix == 'd')
                                {
                                    long long int llarg = va_arg(args, long long int);
                                    const TypeMacros::u32 length = sprintf(smallBuffer, "%lld", llarg);
                                    SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                                    count += length;
                                }
                            }
                        }
                    }
                }
                else if (*format == 'h')
                {
                    char postfix = *(format + 1);
                    if (is_space(postfix))
                    {
                        short sharg = static_cast<short>(va_arg(args, int));
                        const TypeMacros::u32 length = sprintf(smallBuffer, "%hd", sharg);
                        SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                        count += length;
                    }
                    else
                    {
                        format++;

                        if (postfix == 'd')
                        {
                            short int sharg = static_cast<short int>(va_arg(args, int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hd", sharg);
                            SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'u')
                        {
                            unsigned short int sharg = static_cast<unsigned short int>(va_arg(args, int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hu", sharg);
                            SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                    }
                }
                else if (*format == 'b')
                {
                    char postfix = *(format + 1);
                    if (is_space(postfix))
                    {
                        char barg = static_cast<char>(va_arg(args, int));
                        const TypeMacros::u32 length = sprintf(smallBuffer, "%hhd", barg);
                        SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                        count += length;
                    }
                    else
                    {
                        format++;
                        if (postfix == 'd')
                        {
                            char barg = static_cast<char>(va_arg(args, int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hhd", barg);
                            SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                        else if (postfix == 'u')
                        {
                            unsigned char barg = static_cast<unsigned char>(va_arg(args, unsigned int));
                            const TypeMacros::u32 length = sprintf(smallBuffer, "%hhu", barg);
                            SystemIO::standardOut->write(reinterpret_cast<const TypeMacros::u8*>(smallBuffer), length);
                            count += length;
                        }
                    }
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

    TypeMacros::u32 available()
    {
        if (SystemIO::standardInp == nullptr)
            SystemIO::standardInp = __filetable.get_handle(SYSIN_NAME);

        assert(SystemIO::standardInp != nullptr && "[SystemIO::available()] FATAL ERROR: Standard Input is not defined");

        return SystemIO::standardInp->available();
    }

    void printchar(const TypeMacros::u8 c)
    {
        if (SystemIO::standardOut == nullptr)
            SystemIO::standardOut = __filetable.get_handle(SYSOUT_NAME);

        assert(SystemIO::standardOut != nullptr && "[SystemIO::printchar(const char)] FATAL ERROR: Standard Output is not defined");

        SystemIO::standardOut->write(c);
    }

    void perror(const char* str)
    {
        if (SystemIO::standardErr == nullptr)
            SystemIO::standardErr = __filetable.get_handle(SYSERR_NAME);

        assert(SystemIO::standardErr != nullptr && "[SystemIO::perror(const char*)] FATAL ERROR: Standard Error is not defined");

        SystemIO::standardErr->write( reinterpret_cast<const TypeMacros::u8*>(str), strlen(str) );
    }

    void flush()
    {
        if (SystemIO::standardOut == nullptr)
            SystemIO::standardOut = __filetable.get_handle(SYSOUT_NAME);

        assert(SystemIO::standardOut != nullptr && "[SystemIO::printchar(const char)] FATAL ERROR: Standard Output is not defined");

        SystemIO::standardOut->flush();
    }
};