#include <Arduino.h>
#include <OperatingSystem.hpp>

OperatingSystem os;

void setup()
{
    Serial.begin(115200);
    os.setup();
}

void loop() {
}