#include <Arduino.h>

#include "TypeMacros.hpp"
#include "Allocator.hpp"
#include "IOStreams.hpp"
#include "DataStructures.hpp"
#include "Application.hpp"
#include "SystemApplications.hpp"
#include "CommandLine.hpp"

// Global Const variables

// Standard Output Stream (Uses the Serial interface to print data onto the screen)
StandardOutput outputStream;

// Standard Input Stream (Uses the Serial interface to read data from the UART)
StandardInput inputStream;

// Standard Error Stream (is not implemented, for now)
StandardError errorStream;

void loadEverything();

void setup()
{
    Serial.begin(115200);
    loadEverything();
}

void loop()
{
    // Keep empty to avoid repeating the demo continuously
}

void loadEverything()
{
    allFiles.push(&outputStream);
    allFiles.push(&inputStream);
    allFiles.push(&errorStream);
}

void prepareSpecialMemorySpace()
{
}