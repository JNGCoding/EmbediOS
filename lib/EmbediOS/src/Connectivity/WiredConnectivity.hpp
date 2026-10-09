#ifndef __EMBEDI_SERIAL_MANAGER_HPP__
#define __EMBEDI_SERIAL_MANAGER_HPP__

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "../TypeMacros.hpp"
#include "../FileSystem/IOStreams.hpp"

constexpr TypeMacros::u32 DEFAULT_SERIAL = 0;

enum class BitOrder
{
    MSB_FIRST,
    LSB_FIRST
};

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
enum class ChannelType
{
    SENDER,
    RECEIVER
};

struct EmbediWireConfig
{
    TypeMacros::u32 speed;
    ChannelType channel_type;
    TypeMacros::u8 recvaddr;
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
    BitOrder bitOrder;
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
    static void init_i2c(EmbediWireConfig config)
    {
        if (config.channel_type == ChannelType::SENDER)
        {
            Wire.begin();
            Wire.setClock(config.speed);
        }
        else
        {
            Wire.begin(config.recvaddr);
        }

        initialized = true;
    }

    static void deinit_i2c()
    {
        Wire.end();
        initialized = false;
    }

    static const bool is_initialized()
    {
        return initialized;
    }

private:
    static bool initialized;
};

// This struct provides an easy access to setup the SPI Serial channels of the hardware
// Allowing for quick startup
struct EmbediSPIManager
{
    static void init_spi(EmbediSPIConfig config)
    {
        SPI.begin(config.sck, config.miso, config.mosi);
        SPI.setFrequency(config.speed);
        SPI.setBitOrder((config.bitOrder == BitOrder::MSB_FIRST ? MSBFIRST : LSBFIRST));

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

        globalConfiguration = config;
    }

    static void deinit_spi()
    {
        SPI.end();
    }

    static const EmbediSPIConfig get_global_config()
    {
        return globalConfiguration;
    }

private:
    static EmbediSPIConfig globalConfiguration;
};

constexpr TypeMacros::u32 WireStream_BlockThreshold = 50;

class EmbediWireStream : public EmbediFileStream
{
private:
    const TypeMacros::u8 addr;

public:
    EmbediWireStream(const char* name, const TypeMacros::u8 _addr, const TypeMacros::u32 bufferSize);
    bool open() override;
    void post(const TypeMacros::u32 n = IO_POST_SPEED) override;
    void poll() override;

    // Requests one byte from the device, after setting (writing) `reg` of the device
    // It is blocking, with a ms timeout threshold of `WireStream_BlockThreshold`
    // Holds the databus online (calls Wire.endTransmission(false))
    TypeMacros::u8 request(const TypeMacros::u8 reg);

    // Requests one byte from the device
    // It is blocking, with a ms timeout threshold of `WireStream_BlockThreshold`
    TypeMacros::u8 request();

    // Requests `num` bytes from the device, after setting (writing) `reg` of the device
    // It is blocking, with a ms timeout threshold of `WireStream_BlockThreshold`
    // Holds the databus online (calls Wire.endTransmission(false))
    TypeMacros::u32 requestBytes(const TypeMacros::u8 reg, TypeMacros::u8* buf, const TypeMacros::u8 num);
};

class EmbediSPIStream : public EmbediFileStream
{
private:
    const TypeMacros::u8 cs;
    const bool csActive;

    EmbediSPIConfig config = EmbediSPIManager::get_global_config();

    void activate_device();
    void deactivate_device();

public:
    EmbediSPIStream(const char* name, const TypeMacros::u8 _cs, const bool _active_sig, const TypeMacros::u32 bufferSize);
    void set_config(EmbediSPIConfig config);
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