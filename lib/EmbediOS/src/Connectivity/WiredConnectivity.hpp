#ifndef __EMBEDI_SERIAL_MANAGER_HPP__
#define __EMBEDI_SERIAL_MANAGER_HPP__

#include <Arduino.h>
#include "TypeMacros.hpp"

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
};

// This struct provides an easy access to setup the Serial channels of the hardware
// Allowing for quick startup
struct EmbediSerialManager
{
public:
    static void init_serial(const TypeMacros::u8 index, EmbediSerialConfig config)
    {
        if (index == 0)
        {
            Serial.begin(config.speed, config.configuration, config.TX, config.RX);
        }
        else if (index == 1)
        {
            Serial2.begin(config.speed, config.configuration, config.TX, config.RX);
        }
    }

    static void deinit_serial(const TypeMacros::u8 index)
    {
        if (index == 0)
        {
            Serial.end();
        }
        else if (index == 1)
        {
            Serial2.end();
        }
    }
};

// Default Serial STD Output for ease of use
struct EmbediSerialSystemOutput : public EmbediFileStream
{
    EmbediSerialSystemOutput() { this->descriptor = {SYSOUT_NAME, "", WRITE}; }

    bool write(const TypeMacros::u8 _byte) override;
    TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size) override;
    void flush() override;
};

// Default Serial STD Input for ease of use
struct EmbediSerialSystemInput : public EmbediFileStream
{
    EmbediSerialSystemInput() { this->descriptor = {SYSIN_NAME, "", READ }; }
    
    TypeMacros::u8 read() override;
    TypeMacros::u32 read(TypeMacros::u8* buffer, TypeMacros::u32 size) override;
    TypeMacros::u32 available() override;
};

#endif