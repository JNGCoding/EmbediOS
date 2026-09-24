#include <Arduino.h>

#include <OS_Setup.hpp>
#include <Connectivity/WiredConnectivity.hpp>

EmbediSerialSystemOutput out;
EmbediSerialSystemInput inp;
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