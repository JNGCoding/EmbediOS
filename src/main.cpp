#include <Arduino.h>

#include <OperatingSystem.hpp>
#include <OS_Setup.hpp>

#include <Connections/WiredConnectivity.hpp>
#include <FileSystem/IOStreams.hpp>

OperatingSystem OS;

EmbediSerialSystemOutput out;
EmbediSerialSystemInput inp;
EmbediDummySystemError err;

void setup()
{
    EmbediSerialManager::init_serial(DEFAULT_SERIAL, EmbediSerialConfig{DEFAULT_SERIAL, SERIAL_8N1, -1, -1});
    setup_streams(&out, &inp, &err);
}

void loop() {
}