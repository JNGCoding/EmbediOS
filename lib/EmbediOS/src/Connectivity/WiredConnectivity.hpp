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
    bool inverse;
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

class EmbediUARTStream : public EmbediFileStream
{
private:
    HardwareSerial& serial = Serial;

public:
    EmbediUARTStream(const TypeMacros::u8 index, const TypeMacros::u32 bufferSize);
    bool open() override;
    void post(const TypeMacros::u32 n = IO_POST_SPEED) override;
    void poll() override;
};

#endif