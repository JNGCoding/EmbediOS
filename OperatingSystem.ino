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

So few programs can be compiled and embedded into the system itself.
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
#include <string.h>

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

// Command Line Statement
char CLS[MAX_STATEMENT_LENGTH] = {0};

// Virtual Machine for executing program
VirtualMachine VM;
u8 Program[ MAX_PROGRAM_SIZE ] = {0};

#ifdef COMPILE_WITH_SD_INITIALIZATION

// Current Working Directory
EmbediString CWD("/");

int listdir(int argc, const char* argv[])
{
    SdFile dir;

    bool print_size = false;

    for (int i = 0; i < argc; i++)
    {
        if (string_equals(argv[0], "-psize"))
        {
            print_size = true;
        }
    }

    const char* path = CWD.c_str();
    SystemIO::printf("Printing Contents of %s\n", path);

    dir.open(path);

    char nameBuffer[64];

    if (!dir.isDir())
        return -1;

    dir.rewind();

    SdFile file;
    while (file.openNext(&dir, O_READ))
    {
        file.getName(nameBuffer, 64);

        SystemIO::printf("%s");

        if (file.isDir()) {
            SystemIO::printf("\t<DIR>\n");
        } else {
            SystemIO::printf("\t<FIL>");

            if (print_size)
            {
                TypeMacros::usize bytes = 0, bytesAvailable = 0;
                while ((bytesAvailable = file.available()) > 0)
                {
                    TypeMacros::usize incrementer = __max(24, bytesAvailable);
                    bytes += incrementer;
                    file.seekSet( file.curPosition() + incrementer );
                }

                SystemIO::printf("\t%u bytes", static_cast<unsigned int>(bytes));
            }
            SystemIO::printchar('\n');
        }

        file.close();
    }

    dir.close();

    return 0;
}

int run_program(int argc, const char* argv[])
{
    if (argc == 0)
    {
        SystemIO::printf("no file was specified, exiting...");
        return -1;
    }

    SystemIO::printf("Loading program...\n");

    SDCardFileStream file(argv[0], READ);
    bool opened = SystemIO::open_file(&file);
    if (!opened)
    {
        SystemIO::printf("Failed to open file, exiting...\n");
        return -1;
    }

    u32 appendIndex = 0;
    while (file.available() > 0)
    {
        Program[ appendIndex++ ] = file.read();
        if (appendIndex >= MAX_PROGRAM_SIZE)
        {
            SystemIO::printf("error: program size is greater than size limit: %u bytes, exiting...\n", MAX_PROGRAM_SIZE);
            return -1;
        }
    }

    SystemIO::close_file(&file);

    SystemIO::printf("Program successfully read, program size: %u bytes\n", appendIndex);

    VM.load_program(Program, appendIndex, &basicAllocator);
    VM.load_streams(&outputStream, &inputStream);
    int rs = VM.start_program();
    VM.register_dump();

    SystemIO::printf("Program exited with return code = %d\n", rs);

    return 0;
}

int print_file(int argc, const char* argv[])
{
    if (argc <= 0)
    {
        SystemIO::printf("No input file specified, exiting...\n");
        return -1;
    }

    SDCardFileStream stream(argv[0], READ);
    if (!SystemIO::open_file(&stream))
    {
        SystemIO::printf("Failed to open file %s\n", argv[0]);
        return -1;
    }

    TypeMacros::u8 buffer[256];
    while (stream.available() > 0)
    {
        TypeMacros::u32 bytesRead = stream.read(buffer, 256);
        outputStream.write(buffer, bytesRead);
    }

    SystemIO::close_file(&stream);
    
    return 0;
}

#endif

CommandLine CMD;

void loadEverything();
void runCMDFunction(CommandLine::FunctionPackage* f);

void setup()
{
    Serial.begin(115200);
    loadEverything();

    SystemIO::printf("/ : ");
}

void loop()
{
    u32 bytesRead = SystemIO::gets(CLS, MAX_STATEMENT_LENGTH);
    if (bytesRead <= 0)
        return;

    // We are going to trim the '\r\n'
    CLS[bytesRead] = '\0';

    SystemIO::printf("%s\n", CLS);

    CommandLine::FunctionPackage* function = CMD.process_statement(CLS);

    if (function != nullptr) {
        runCMDFunction(function);
    } else {
        SystemIO::printf("%s is not valid command\n", CLS);
    }

    SystemIO::printf("%s : ", CWD.c_str());
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
    else
    {
        SystemIO::printf("SUCCESS: INITIALIZED SD CARD AND FAT32 FILE SYSTEMS\n");
    }

    CMD.register_function("listdir", listdir);
    CMD.register_function("start", run_program);
    CMD.register_function("printfile", print_file);

#endif
}

void runCMDFunction(CommandLine::FunctionPackage* f)
{
    f->function(f->argc, f->argv);
}