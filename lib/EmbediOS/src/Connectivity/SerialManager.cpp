#include "WiredConnectivity.hpp"

EmbediUARTStream::EmbediUARTStream(const char* name, const TypeMacros::u8 index, const TypeMacros::u32 bufferSize) : EmbediFileStream((bufferSize == 0 ? 1024 : bufferSize))
{
    if (bufferSize == 0)
        SystemIO::perror("[EmbediUARTStream::EmbediUARTStream(u8, u32)] WARNING: bufferSize provided is 0, defaulting to 1024 bytes size buffers.");

    if (index == 0)      { this->serial = Serial;  }
    else if (index == 1) { this->serial = Serial2; }

    this->descriptor = {
        name,
        ".sys/uart/io/serial_stream",
        WRITE | READ
    };
}

bool EmbediUARTStream::open()
{
    this->isOpened = this->allocate_buffers();
    return this->isOpened;
}

void EmbediUARTStream::post(const TypeMacros::u32 n)
{
    if (this->tx_ready())
    {
        const TypeMacros::u32 toPost = embedi_min(this->writeQueue.size(), n);
        for (TypeMacros::u32 i = 0; i < toPost; i++)
        {
            TypeMacros::u8* data = this->writeQueue.dequeue();
            if (data == nullptr)
            {
                this->error = ErrorCode::NULLPTR_ENCOUNTERED;
            }
            else
            {
                this->error = ErrorCode::NO_ERROR;
                this->serial.write(*data);
            }
        }
    }
    else this->error = ErrorCode::WRITE_BUFFER_NULL;
}

void EmbediUARTStream::poll()
{
    if (this->rx_ready())
    {
        while (this->serial.available() > 0)
            this->readRingBuffer.push(this->serial.read());
    } else this->error = ErrorCode::READ_BUFFER_NULL;
}