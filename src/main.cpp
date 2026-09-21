#include <Arduino.h>

#include "FileSystem/IOStreams.hpp"

SdFat SDCardFileStream::card;

void setup()
{
    Serial.begin(115200);
}

void loop() {
}