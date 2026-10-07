#include "WiredConnectivity.hpp"

EmbediWireStream::EmbediWireStream(const char* name, const TypeMacros::u8 _addr, const TypeMacros::u32 bufferSize) : addr(_addr), EmbediFileStream((bufferSize != NOT_BUFFERED ? 1024 : bufferSize), true, false)
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

void EmbediWireStream::poll()
{}

TypeMacros::u8 EmbediWireStream::request(const TypeMacros::u8 reg)
{
    Wire.beginTransmission(this->addr);
    Wire.write(reg);    
    Wire.endTransmission(false);

    Wire.requestFrom(this->addr, static_cast<TypeMacros::u8>(1));

    TypeMacros::u32 start = millis();

    while (millis() - start <= WireStream_BlockThreshold)
    {
        if (Wire.available() > 0)
        {
            this->error = ErrorCode::NO_ERROR;
            return Wire.read();
        }
    }

    this->error = ErrorCode::TIMEOUT;
    return 0;
}

TypeMacros::u8  EmbediWireStream::request()
{
    Wire.requestFrom(this->addr, static_cast<TypeMacros::u8>(1));
    TypeMacros::u32 start = millis();

    while (millis() - start <= WireStream_BlockThreshold)
    {
        if (Wire.available() > 0)
        {
            this->error = ErrorCode::NO_ERROR;
            return Wire.read();
        }
    }

    this->error = ErrorCode::TIMEOUT;
    return 0;
}

TypeMacros::u32 EmbediWireStream::requestBytes(const TypeMacros::u8 reg, TypeMacros::u8* buf, const TypeMacros::u8 num)
{
    Wire.beginTransmission(this->addr);
    Wire.write(reg);    
    Wire.endTransmission(false);

    Wire.requestFrom(this->addr, num);

    TypeMacros::u32 start = millis();
    TypeMacros::u32 appind = 0;

    while (millis() - start <= WireStream_BlockThreshold && appind < num)
    {
        if (Wire.available() > 0)
        {
            this->error = ErrorCode::NO_ERROR;
            buf[appind++] = Wire.read();
            start = millis();
        }
    }

    return appind;
}
