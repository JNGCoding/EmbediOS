#include "WiredConnectivity.hpp"

// Standard IO Stream implementation
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

TypeMacros::u8 EmbediSerialSystemInput::read()
{
    return Serial.read();
}

TypeMacros::u32 EmbediSerialSystemInput::read(TypeMacros::u8* buffer, TypeMacros::u32 size)
{
    return Serial.readBytes(buffer, size);
}

TypeMacros::u32 EmbediSerialSystemInput::available()
{
    return Serial.available();
}