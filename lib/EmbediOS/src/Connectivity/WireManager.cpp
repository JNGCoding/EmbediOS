#include "WiredConnectivity.hpp"

EmbediWireStream::EmbediWireStream(const char* name, const TypeMacros::u8 _addr, const TypeMacros::u32 bufferSize) : addr(_addr), EmbediFileStream((bufferSize != NOT_BUFFERED ? 1024 : bufferSize))
{
    if (bufferSize == NOT_BUFFERED)
        SystemIO::perror("[EmbediWireStream::EmbediWireStream(u8, u32)] ERROR: BUFFER_SIZE PROVIDED WAS 0, DEFAULTING TO 1024 BYTES.");

    this->descriptor = {
        name,
        ".sys/wire/io/serial_streams",
        WRITE | READ
    };
}

bool EmbediWireStream::open()
{
    this->isOpened = this->allocate_buffers();
    return this->isOpened;
}

void EmbediWireStream::post(const TypeMacros::u32 n)
{
    if (this->tx_ready() && this->opened())
    {
        const TypeMacros::u32 toPost = embedi_min(this->writeQueue.size(), n);

        Wire.beginTransmission(this->addr);
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
                Wire.write(*data);
            }
        }

        Wire.endTransmission();
    }
    else this->error = ErrorCode::WRITE_BUFFER_NULL;
}

void poll()
{}