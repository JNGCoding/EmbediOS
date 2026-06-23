/*
@author JNGCoding aka Dhruv

@note
I don't like to sugarcoat a thing or two so first of all
This operating system will be slow.

Most of the memory will be statically allocated and dynamically allocated memory will
be maintained through special allocators.

This system upon just reading is very easy to understand.
I mean most of the things that I implemented are just abstractions and Useless data structures

But They are created in order to provide flexibility and that comes with a cost

I mean you can't really call it an operating system.
It will be more like a virtual machine sitting on top of some hardware that you
can find.

and it is, Programs that can be loaded from the SD Card are actually byte code instructions
for the Virtual Machine that will be running them.

But programs can be embedded into the system as well. Again Abstractions provides
flexibility.

So few programs can be compiled and loaded into the system.
The number of program that can be compiled and loaded depends on the memory and
flash storage of the Micro-Controller that you are currently holding.

I will not be creating any special bootloaders and such, that is the reason I chose
Arduino Framework to create this project under because you know.... Already developed
bootloaders for me to use >3<

Anyways, I am not egoist enough to keep this project to myself. I want to see what
the community can come up with this shitty software.
*/

#include <Arduino.h>
#include <SdFat.h>

#include "TypeMacros.hpp"
#include "Allocator.hpp"
#include "IOStreams.hpp"
#include "DataStructures.hpp"
#include "Application.hpp"
#include "SystemApplications.hpp"
#include "CommandLine.hpp"
#include "Defines.hpp"
#include "VirtualMachine.hpp"

using namespace TypeMacros;

StandardOutput outputStream;
StandardInput inputStream;
StandardError errorStream;
SdFat SDCardFileStream::card;

constexpr usize MAX_STATEMENT_LENGTH = 1024;
char commandLineStatement[MAX_STATEMENT_LENGTH] = {0};

constexpr TypeMacros::u8 SD_CARD_CHIP_SELECT = 5;

CommandLine CMD;

void loadEverything();
void runCMDFunction(CommandLine::FunctionPackage* f);

u8 program[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0xfd, 0x01, 0x02, 0x00, 0x00, 0x00, 0x48, 0x00, 0x01, 0x01, 0x02, 0x00, 0x00, 0x00, 0x48, 0x00, 0x01, 0x13, 0x01, 0x02, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x13, 0x03, 0x02, 0x00, 0x00, 0x00, 0x10, 0x14};
u8 program_heap[Kb(10)] = {0};

VirtualMachine machine;

void setup()
{
    Serial.begin(115200);
    loadEverything();

    u32 progSize = sizeof(program) / sizeof(u8);
    SystemIO::printf("Program size: %u\n", progSize);

    machine.load_program(program, progSize, program_heap);
    int rc = machine.start_program();
    machine.register_dump();

    SystemIO::printf("VirtualMachine returned with %d\n", rc);
}

void loop()
{
    u32 bytesRead = SystemIO::gets(commandLineStatement, MAX_STATEMENT_LENGTH);
    if (bytesRead <= 0)
        return;

    // We are going to trim the '\r\n'
    commandLineStatement[bytesRead] = '\0';

    SystemIO::printf("Statement: %s, length: %u\n", commandLineStatement, strlen(commandLineStatement));

    CommandLine::FunctionPackage* function = CMD.process_statement(commandLineStatement);

    if (function != nullptr) {
        runCMDFunction(function);
    } else {
        SystemIO::printf("Function recieved is nullptr\n");
    }
}

void loadEverything()
{
    // Load the default streams
    allFiles.push(&outputStream);
    allFiles.push(&inputStream);
    allFiles.push(&errorStream);

#ifdef COMPILE_WITH_SD_INITIALIZATION

    // Load the SdFat variables to intialize SdCard Streams
    if (!SDCardFileStream::card.begin(SD_CARD_CHIP_SELECT, SD_SCK_MHZ(8)))
    {
        SystemIO::printf("[SDCardFileStream::card.begin()] FATAL ERROR: FAILED TO INITIALIZE SD CARD");
        while (1) yield();
    }

#endif
}

void runCMDFunction(CommandLine::FunctionPackage* f)
{
    f->function(f->argc, f->argv);
}