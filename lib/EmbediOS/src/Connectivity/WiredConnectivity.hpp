#ifndef __EMBEDI_SERIAL_MANAGER_HPP__
#define __EMBEDI_SERIAL_MANAGER_HPP__

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "../TypeMacros.hpp"
#include "../FileSystem/IOStreams.hpp"

constexpr TypeMacros::u32 DEFAULT_SERIAL = 0;

// Serial Config object
// Holds configuration for initializing the serial
struct EmbediSerialConfig
{
    TypeMacros::u32 speed;
    TypeMacros::u32 configuration;
    TypeMacros::i8 TX;
    TypeMacros::i8 RX;
    bool inverse;
};

// Wire Config object
// Holds configuration for initializing the i2c
enum class WireChannelType
{
    SENDER,
    RECEIVER
};

struct EmbediWireConfig
{
    TypeMacros::u32 speed;
    WireChannelType channel_type;
    TypeMacros::u8 addr;
};

// SPI Config object
// Holds configuration for initializing the spi
enum class SPIMode
{
    Mode0,  // CPOL = 0, CPHA = 0
    Mode1,  // CPOL = 0, CPHA = 1
    Mode2,  // CPOL = 1, CPHA = 0
    Mode3   // CPOL = 1, CPHA = 1
};

struct EmbediSPIConfig
{
    TypeMacros::u32 speed;
    SPIMode mode;
    TypeMacros::u8 mosi, miso, sck;
};

// This struct provides an easy access to setup the Serial channels of the hardware
// Allowing for quick startup
struct EmbediSerialManager
{
public:
    static void init_serial(const TypeMacros::u8 index, EmbediSerialConfig config)
    {
        switch (index)
        {
            case 0:
                Serial.begin(config.speed, config.configuration, config.TX, config.RX, config.inverse);
                break;

            case 1:
                Serial2.begin(config.speed, config.configuration, config.TX, config.RX, config.inverse);
                break;

            default: {
                EmbediFileStream* s = __filetable.get_handle(SYSERR_INDEX);
                if (s != nullptr)
                    SystemIO::fprintf(s, "Can't initialize a Serial port with index %u", index);
            } break;
        }
    }

    static void deinit_serial(const TypeMacros::u8 index)
    {
        switch (index)
        {
            case 0:
                Serial.end();
                break;

            case 1:
                Serial2.end();
                break;

            default: {
                EmbediFileStream* s = __filetable.get_handle(SYSERR_INDEX);
                if (s != nullptr)
                    SystemIO::fprintf(s, "Can't deinitialize a Serial port with index %u", index);
            } break;
        }
    }
};

// This struct provides an easy access to setup the I2C Serial channels of the hardware
// Allowing for quick startup
struct EmbediWireManager
{
    static void init_i2c(EmbediWireConfig& config)
    {
        if (config.channel_type == WireChannelType::SENDER)
        {
            Wire.begin();
            Wire.setClock(config.speed);
        }
        else
        {
            Wire.begin(config.addr);
        }
    }

    static void deinit_i2c()
    {
        Wire.end();
    }
};

// This struct provides an easy access to setup the SPI Serial channels of the hardware
// Allowing for quick startup
struct EmbediSPIManager
{
    static void init_spi(EmbediSPIConfig config)
    {
        SPI.begin(config.sck, config.miso, config.mosi);
        SPI.setFrequency(config.speed);

        switch (config.mode)
        {
            case SPIMode::Mode0:
                SPI.setDataMode(SPI_MODE0);
                break;
            
            case SPIMode::Mode1:
                SPI.setDataMode(SPI_MODE1);
                break;

            case SPIMode::Mode2:
                SPI.setDataMode(SPI_MODE2);
                break;

            case SPIMode::Mode3:
                SPI.setDataMode(SPI_MODE3);
                break;
            
            default:
                SystemIO::perror("[EmbediSPIManager::init_spi(EmbediSPIConfig)] ERROR: Defaulting to SPI_MODE0.");
                SPI.setDataMode(SPI_MODE0);
                break;
        }
    }

    static void deinit_spi()
    {
        SPI.end();
    }
};

class EmbediWireStream : public EmbediFileStream
{
private:
    const TypeMacros::u8 addr;

public:
    EmbediWireStream(const char* name, const TypeMacros::u8 _addr, const TypeMacros::u32 bufferSize);
    bool open() override;
    void post(const TypeMacros::u32 n = IO_POST_SPEED) override;
    void poll() override;
};

class EmbediSPIStream : public EmbediFileStream
{
private:
    const TypeMacros::u8 cs_pin;

public:
    EmbediSPIStream(const char* name, const TypeMacros::u8 _cs, const TypeMacros::u32 bufferSize);
    bool open() override;
    void post(const TypeMacros::u32 n = IO_POST_SPEED) override;
    void poll() override;
};

class EmbediUARTStream : public EmbediFileStream
{
private:
    HardwareSerial& serial = Serial;

public:
    EmbediUARTStream(const char* name, const TypeMacros::u8 index, const TypeMacros::u32 bufferSize);
    bool open() override;
    void post(const TypeMacros::u32 n = IO_POST_SPEED) override;
    void poll() override;
};

#endif