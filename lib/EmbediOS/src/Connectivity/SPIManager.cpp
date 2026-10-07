#include "WiredConnectivity.hpp"

EmbediSPIConfig EmbediSPIManager::globalConfiguration;

inline void EmbediSPIStream::activate_device()
{ digitalWrite(this->cs, (this->csActive ? HIGH : LOW)); }

inline void EmbediSPIStream::deactivate_device()
{ digitalWrite(this->cs, (this->csActive ? LOW : HIGH)); }

EmbediSPIStream::EmbediSPIStream(const char* name, const TypeMacros::u8 _cs, const bool cs_active, const TypeMacros::u32 bufferSize) : cs(_cs), csActive(cs_active), EmbediFileStream((bufferSize != NOT_BUFFERED ? bufferSize : 1024), true, false)
{
    this->descriptor = {
        name,
        ".sys/spi/io/serial_streams",
        READ | WRITE
    };

    pinMode(this->cs, OUTPUT);
    this->deactivate_device();

    this->config = EmbediSPIManager::get_global_config();
}

void EmbediSPIStream::set_config(EmbediSPIConfig config)
{
    this->config = config;
}

bool EmbediSPIStream::open()
{
    this->isOpened = this->allocate_buffers();
    return this->isOpened;
}

void EmbediSPIStream::post(const TypeMacros::u32 n)
{
    if (this->tx_ready() && this->opened())
    {
        const TypeMacros::u32 toPost = embedi_min(this->writeQueue.size(), n);

        int mode = SPI_MODE0;
        switch (this->config.mode)
        {
            case SPIMode::Mode0:
                mode = SPI_MODE0;
                break;
            case SPIMode::Mode1:
                mode = SPI_MODE1;
                break;
            case SPIMode::Mode2:
                mode = SPI_MODE2;
                break;
            case SPIMode::Mode3:
                mode = SPI_MODE3;
                break;
            default:
                mode = SPI_MODE0;
                break;
        }
        SPI.beginTransaction(SPISettings{this->config.speed, (this->config.bitOrder == BitOrder::MSB_FIRST ? MSBFIRST : LSBFIRST), mode});
        this->activate_device();

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
                TypeMacros::u8 response = SPI.transfer(*data);
                this->readRingBuffer.push(response);
            }
        }

        this->deactivate_device();
        SPI.endTransaction();
    }
    else this->error = ErrorCode::WRITE_BUFFER_NULL;
}

void EmbediSPIStream::poll()
{}