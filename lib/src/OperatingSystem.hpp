#ifndef __EMBEDI_OS_OPERATING_SYSTEM_HPP__
#define __EMBEDI_OS_OPERATING_SYSTEM_HPP__

#include <Arduino.h>

class OperatingSystem
{
private:
public:
    OperatingSystem()
    {}

    void setup()
    {
        Serial.println("Setup working successfully");
    }
};

#endif