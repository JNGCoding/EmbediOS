#include <Arduino.h>

#include <OS_Setup.hpp>
#include <Connectivity/WiredConnectivity.hpp>

EmbediSerialSystemOutput out(DEFAULT_SERIAL);
EmbediSerialSystemInput inp(DEFAULT_SERIAL);
EmbediDummySystemError err;

void setup()
{
    EmbediSerialManager::init_serial(DEFAULT_SERIAL, EmbediSerialConfig{115200, SERIAL_8N1, -1, -1});
    setup_streams(&out, &inp, &err);

    load_starter_functions();
}

void loop()
{
    run_basic_loop_iteration();
}