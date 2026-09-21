#include <Arduino.h>

#include <OperatingSystem.hpp>
#include <OS_Setup.hpp>

#include <Connections/WiredConnectivity.hpp>

OperatingSystem OS;

void setup()
{

    EmbediSerialManager::init_serial(DEFAULT_SERIAL, EmbediSerialConfig{0, 0, 0, 0});
    load_starter_functions();
}

void loop() {
}