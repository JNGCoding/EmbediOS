#include "WirelessConnectivity.hpp"

#ifdef COMPILE_WITH_BLUETOOTH_INITIALIZATION

BluetoothSerial __Internal_BTSerial;
EmbediBluetoothConfig __Internal_config = {0};
bool __Internal_BluetoothActivated = false;

EmbediBluetoothSerialOutput::EmbediBluetoothSerialOutput() : EmbediFileStream(NOT_BUFFERED)
{
    this->descriptor =
    {
        SYSOUT_NAME,
        "Bluetooth/Serial/",
        WRITE
    };
}

EmbediBluetoothSerialOutput::~EmbediBluetoothSerialOutput()
{
    this->flush();
}

bool EmbediBluetoothSerialOutput::write(const TypeMacros::u8 _byte)
{
    if (!__Internal_BluetoothActivated)
        return false;

    return __Internal_BTSerial.write(_byte) != 0;
}

void EmbediBluetoothSerialOutput::flush()
{
    if (!__Internal_BluetoothActivated)
        return;

    __Internal_BTSerial.flush();
}

EmbediBluetoothSerialInput::EmbediBluetoothSerialInput() : EmbediFileStream(NOT_BUFFERED)
{
    this->descriptor =
    {
        SYSIN_NAME,
        "Bluetooth/Serial/",
        READ
    };
}

TypeMacros::u8 EmbediBluetoothSerialInput::read()
{
    if (!__Internal_BluetoothActivated)
        return 0;

    return __Internal_BTSerial.read();
}

TypeMacros::u32 EmbediBluetoothSerialInput::available()
{
    if (!__Internal_BluetoothActivated)
        return 0;

    return __Internal_BTSerial.available();
}

#endif