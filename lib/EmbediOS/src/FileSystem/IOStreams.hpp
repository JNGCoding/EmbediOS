#ifndef __EMBEDI_IO_STREAMS_HPP__
#define __EMBEDI_IO_STREAMS_HPP__

#include <Arduino.h>
#include <string.h>
#include <stdarg.h>

#include "Defines.hpp"

#ifdef COMPILE_WITH_SD_INITIALIZATION
#include <SdFat.h>
#endif

#include "../TypeMacros.hpp"
#include "../Allocator/Allocator.hpp"
#include "../DataStructures/DataStructures.hpp"

#ifdef __AVR__
#undef getchar
#endif

// At max how many files can be opened in the operating system simultaneously
//
// First 3 positions are reserved by sysout, sysin, syserr
constexpr TypeMacros::u32 MAX_FILE_HANDLES = 10;

// Avialable File Modes
constexpr TypeMacros::u8 READ   = 1 << 0;
constexpr TypeMacros::u8 WRITE  = 1 << 1;
constexpr TypeMacros::u8 APPEND = 1 << 2 | WRITE;

// Name of System output, input and error streams respectively
constexpr const char* SYSOUT_NAME = "stdout";
constexpr const char* SYSIN_NAME =  "stdin";
constexpr const char* SYSERR_NAME = "stderr";

// This is an identifier object used for getting any filestream from FileTable
// instance, it contains credentials
// which is used to clarify what functions of the file can actually be used
struct FileDescriptor
{
    const char* name;
    const char* path;
    TypeMacros::u8 mode;
};

// An interface object for interfacing with various file systems
// This is buffered
constexpr TypeMacros::u32 NOT_BUFFERED = 0U;

class EmbediFileStream
{
protected:
    TypeMacros::u8 *readBuffer = nullptr, *writeBuffer = nullptr;
    EmbediRingBuffer<TypeMacros::u8> readRingBuffer;
    EmbediQueue<TypeMacros::u8> writeQueue;
    const bool buffered;

    constexpr bool tx_ready()
    { return this->writeBuffer != nullptr; }

    constexpr bool rx_ready()
    { return this->readBuffer != nullptr; }

    bool allocate_buffers()
    {
        this->writeBuffer = static_cast<TypeMacros::u8*>(basicAllocator.alloc(this->writeQueue.cap_size()));
        this->readBuffer = static_cast<TypeMacros::u8*>(basicAllocator.alloc(this->writeQueue.cap_size()));

        this->writeQueue.set_buffer(this->writeBuffer);
        this->readRingBuffer.set_buffer(this->readBuffer);

        return this->writeBuffer != nullptr && this->readBuffer != nullptr;
    }

    void destroy_buffers()
    {
        if (this->writeBuffer != nullptr)
            basicAllocator.destroy(this->writeBuffer);

        if (this->readBuffer != nullptr)
            basicAllocator.destroy(this->readBuffer);

        this->writeBuffer = nullptr;
        this->readBuffer = nullptr;

        this->writeQueue.set_buffer(nullptr);
        this->readRingBuffer.set_buffer(nullptr);
    }

public:
    enum class ErrorCode : TypeMacros::u8
    {
        NO_ERROR,
        WRITE_BUFFER_NULL,
        READ_BUFFER_NULL,
        READ_RINGBUFFER_UNDERFLOW,
        WRITE_QUEUE_OVERFLOW
    };

private:
    ErrorCode error;

public:
    FileDescriptor descriptor = {0};

    EmbediFileStream(TypeMacros::u32 bufferSize = NOT_BUFFERED) : buffered(bufferSize != NOT_BUFFERED), readRingBuffer(nullptr, bufferSize), writeQueue(nullptr, bufferSize)
    {
    }

    virtual ~EmbediFileStream()
    { this->close(); }

    // Opens the file stream and allocates the buffers
    virtual bool open()
    { 
        return false;
    }

    // Closes the file stream and frees the buffers allocated associated to that file
    virtual void close()
    {
        if (this->buffered)
        {
            this->flush();
            this->destroy_buffers();
        }
    }

    // This function specializes to do the actual hardware
    // activity of posting something to the device
    // it is printing to
    // `n` bytes are posted to the devices
    // It is blocking
    virtual void post(const TypeMacros::u32 n = IO_POST_SPEED)
    {
        const TypeMacros::u32 toPost = embedi_min(this->writeQueue.size(), n);
        for (TypeMacros::u32 i = 0; i < toPost; i++)
            this->writeQueue.dequeue();

        // We don't have anything to do here
    }

    // This function specializes to do the actual hardware
    // activity of polling something from the device
    // it is reading from
    // It is blocking
    virtual void poll()
    {}

    // Writes some data to the input stream of the file

    // Update now we don't directly write data onto the internal streams of hardware, instead we
    // store data in memory for the sake of buffering and non - blocking behaviour of operating system
    virtual bool write(const TypeMacros::u8 _byte)
    {
        if (this->tx_ready())
        {
            if (this->writeQueue.enqueue(_byte))
            {
                this->error = ErrorCode::NO_ERROR;
                return true;
            }
            else
            {
                this->error = ErrorCode::WRITE_QUEUE_OVERFLOW;
                return false;
            }
        }
        else
        {
            this->error = ErrorCode::WRITE_BUFFER_NULL;
            return false;
        }
    }
    virtual TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size)
    {
        TypeMacros::u32 bytesWritten = 0;
        for (TypeMacros::u32 i = 0; i < size; i++)
        {
            if (!this->write(_data[i]))
                break;
            bytesWritten++;
        }
        return bytesWritten;
    }
    virtual TypeMacros::u32 print(const char* str)
    { return this->write(reinterpret_cast<const TypeMacros::u8*>(str), strlen(str)); }
    virtual TypeMacros::u32 println(const char* str = "")
    { return this->print(str) + (this->write('\r') ? 1 : 0) + (this->write('\n') ? 1 : 0); }

    // Flushes all the data to the input stream of the file
    virtual void flush()
    {
        if (!this->tx_ready())
        {
            this->error = ErrorCode::WRITE_BUFFER_NULL;
            return;
        }

        if (this->writeQueue.size() > 0)
        {
            this->post(this->writeQueue.size());
        }
    }
    
    // Reads one byte from the output stream of file
    virtual TypeMacros::u8 read()
    {
        if (this->rx_ready())
        {
            // Just return the byte
            // pop_byte already sets the error flag
            // if byte is not present
            TypeMacros::u8* s = this->readRingBuffer.pop();
            if (s == nullptr)
            {
                this->error = ErrorCode::READ_RINGBUFFER_UNDERFLOW;
                return 0;
            }

            this->error = ErrorCode::NO_ERROR;
            return *s;
        }

        this->error = ErrorCode::READ_BUFFER_NULL;
        return 0;
    }
    virtual TypeMacros::u32 read(TypeMacros::u8* buffer, TypeMacros::u32 size)
    {
        TypeMacros::u32 bytesRead = 0;
        while (true)
        {
            TypeMacros::u32 bytesAvailable = this->available();
            if (bytesAvailable <= 0 || bytesRead >= size)
                break;

            TypeMacros::u32 toRead = 0;

            if (bytesAvailable + bytesRead > size) {
                toRead = size - bytesRead;
            } else toRead = bytesAvailable;

            for (TypeMacros::u32 i = 0; i < toRead; i++)
                buffer[bytesRead++] = this->read();
        }

        return bytesRead;
    }

    // Changes the position of file pointer relative to the start of the file
    virtual void seek(const TypeMacros::u32 pos) {}

    // Returns the position of the file pointer relative to the start of the file 
    virtual TypeMacros::u32 tell() { return 0; }

    // Returns the no. of available readable bytes in the read buffer
    virtual TypeMacros::u32 available()
    {
        if (this->rx_ready())
        {
            this->error = ErrorCode::NO_ERROR;
            return this->readRingBuffer.size();
        }
        else
        {
            this->error = ErrorCode::READ_BUFFER_NULL;
            return 0;
        }
    }
};

// Stores every opened file stream in its buffer for ease of queuing
struct FileTable
{
    EmbediFileStream* fileHandles[ MAX_FILE_HANDLES ];
    TypeMacros::u32 count = 0;

    // Pushes an EmbediFileStream* onto the buffer
    EmbediFileStream* push(EmbediFileStream* fileStream);

    // Removes an EmbediFileStream* from the buffer referenced by a pointer
    EmbediFileStream* remove(EmbediFileStream* fdesc);

    // Removes an EmbediFileStream* from the buffer referenced by its index
    EmbediFileStream* remove(const int index);

    // Returns an EmbediFileStream* from the buffer reference by its index
    EmbediFileStream* get_handle(const int index);

    // Returns an EmbediFileStream* from the buffer referenced by its name
    // stored in it's descriptor
    EmbediFileStream* get_handle(const char* streamName);

    // Default Post all the handles currently present in the
    // system's FileTable
    void post_all_handles();

    // Default Poll all the handles currently present in the
    // system's FileTable
    void poll_all_handles();
};

// This is the list that will be handled through-out the filespace
extern FileTable& __filetable;

// Empty class for System Error stream
// Since we have exhausted Serial for I/O for user interfacing
// it is made because at a time, it is a must to add
// sysout, sysin, syserr to all streams first for maintaining consistency
struct EmbediDummySystemError : public EmbediFileStream
{
    EmbediDummySystemError() : EmbediFileStream(NOT_BUFFERED)
    { this->descriptor = {SYSERR_NAME, "", READ | WRITE}; }

    bool write(const TypeMacros::u8 _byte) override { return false; }
    TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size) override { return 0; }
    void flush() override {}
    TypeMacros::u8 read() override { return 0; }
    TypeMacros::u32 read(TypeMacros::u8* buffer, TypeMacros::u32 size) override { return 0; }
    TypeMacros::u32 available() override { return 0; }
};

// Implementation of SD Card File Streams
#ifdef COMPILE_WITH_SD_INITIALIZATION
struct SDCardFileStream : public EmbediFileStream
{
    static SdFat card;
    SdFile file;

    SDCardFileStream(const char* path, const TypeMacros::u8 mode);
    ~SDCardFileStream();
    
    bool open() override;
    void close() override;
    bool write(const TypeMacros::u8 _byte) override;
    TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size) override;
    void flush() override;
    TypeMacros::u8 read() override;
    TypeMacros::u32 read(TypeMacros::u8* buffer, TypeMacros::u32 size) override;
    void seek(const TypeMacros::u32 pos) override;
    TypeMacros::u32 tell() override;
    TypeMacros::u32 available() override;

    static bool exists(const char* path)
    {
        return card.exists(path);
    }

    static bool mkdir(const char* path, const bool rooted)
    {
        return card.mkdir(path, rooted);
    }

    static bool rmdir(const char* path)
    {
        return card.rmdir(path);
    }

    static bool create(const char* path)
    {
        if (exists(path))
            return false;

        SdFile file;
        if (!file.open(path, O_CREAT | O_WRITE))
            return false;
        file.close();

        return true;
    }

    static bool remove(const char* path)
    {
        return card.remove(path);
    }
};
#endif

namespace SystemIO {
    bool open_file(EmbediFileStream* fileStream);
    void close_file(EmbediFileStream* fileStream);
    void close_file(const char* streamName);

    TypeMacros::u8 fgetc(const char* streamName);
    TypeMacros::u8 fgetc(EmbediFileStream* stream);

    TypeMacros::u32 favailable(const char* streamName);
    TypeMacros::u32 favailable(EmbediFileStream* stream);

    void fputc(const char* streamName, const TypeMacros::u8 c);
    void fputc(EmbediFileStream* stream, const TypeMacros::u8 c);

    void fflush(const char* streamName);
    void fflush(EmbediFileStream* stream);

    TypeMacros::u32 fputs(const char* streamName, const char* str);
    TypeMacros::u32 fputs(EmbediFileStream* stream, const char* str);

    TypeMacros::u32 fprintf(const char* streamName, const char* format, ...);
    TypeMacros::u32 fprintf(EmbediFileStream* stream, const char* format, ...);

    TypeMacros::u32 fgets(const char* streamName, char* buffer, TypeMacros::u32 capSize, const TypeMacros::u32 block_threshold = 10);
    TypeMacros::u32 fgets(EmbediFileStream* stream, char* buffer, TypeMacros::u32 capSize, const TypeMacros::u32 block_threshold = 10);

    TypeMacros::u8 getchar();

    TypeMacros::u32 available();

    TypeMacros::u32 gets(char* buffer, TypeMacros::u32 capSize, const TypeMacros::u32 block_threshold = 10);
    TypeMacros::u32 printf(const char* format, ...);

    void printchar(const TypeMacros::u8 c);
    void perror(const char* str);

    void flush();
};

#endif