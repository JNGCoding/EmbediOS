#include "WiredConnectivity.hpp"

// Standard IO Stream implementation
EmbediSerialSystemOutput::EmbediSerialSystemOutput(TypeMacros::u32 index)
{
    if (index == 0)      { serial = Serial; }
    else if (index == 1) { serial = Serial2; }

    this->descriptor = {SYSOUT_NAME, "", WRITE};
}

bool EmbediSerialSystemOutput::write(const TypeMacros::u8 _byte)
{
    Serial.write(_byte);
    return true;
}

TypeMacros::u32 EmbediSerialSystemOutput::write(const TypeMacros::u8* _data, TypeMacros::u32 size)
{
    Serial.print(reinterpret_cast<const char*>(_data));
    return size;
}

void EmbediSerialSystemOutput::flush()
{
    Serial.flush();
}

EmbediSerialSystemInput::EmbediSerialSystemInput(TypeMacros::u32 index)
{
    if (index == 0)      { this->serial = Serial; }
    else if (index == 1) { this->serial = Serial2; }

    this->descriptor = {SYSIN_NAME, "", READ };
}


TypeMacros::u8 EmbediSerialSystemInput::read()
{
    return this->serial.read();
}

TypeMacros::u32 EmbediSerialSystemInput::read(TypeMacros::u8* buffer, TypeMacros::u32 size)
{
    return this->serial.readBytes(buffer, size);
}

TypeMacros::u32 EmbediSerialSystemInput::available()
{
    return this->serial.available();
}