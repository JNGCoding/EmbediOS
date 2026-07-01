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

#define MACHINE_RESET_FUNC() ESP.restart()

using namespace TypeMacros;

StandardOutput outputStream;
StandardInput inputStream;
StandardError errorStream;
SdFat SDCardFileStream::card;

// Operational Allocator
// For temporary variables
TypeMacros::u8 operationalMemory[OPERATIONAL_MEMORY_SIZE] = {0};
EmbediLinearAllocator linearAllocator(operationalMemory, OPERATIONAL_MEMORY_SIZE);

// Command Line Statement
char CLS[MAX_STATEMENT_LENGTH] = {0};

// Virtual Machine for executing program
VirtualMachineV2 VM;
u8 Program[ MAX_PROGRAM_SIZE ] = {0};

int reset_machine(int argc, const char* argv[])
{
    MACHINE_RESET_FUNC();
    return 0;
}

int echo(int argc, const char* argv[])
{
    for (int i = 0; i < argc; i++)
        SystemIO::printf("%s ", argv[i]);

    SystemIO::printf("\n");
    SystemIO::flush();
    
    return 0;
}

// Current Working Directory
EmbediString CWD("/");

#ifdef COMPILE_WITH_SD_INITIALIZATION

int change_directory(int argc, const char* argv[])
{
    if (argc <= 0)
    {
        SystemIO::printf("No input directory specified, exiting...\n");
        return -1;
    }

    TypeMacros::u32 len = CWD.length();

    if (string_equals(argv[0], ".."))
    {
        if (string_equals(CWD.c_str(), "/"))
            return 0;

        for (TypeMacros::i32 i = len - 1; i >= 0; i--)
        {
            if (CWD.get_char(i) == '/')
                CWD.set_length(i - 1);  // Remove the '/'
        }

        return 0;
    }

    CWD.append(argv[0]);
    if (CWD.get_char(CWD.length() - 1) != '/')
        CWD.append("/");

    if (!SDCardFileStream::card.exists(CWD.c_str()))
    {
        SystemIO::printf("Path %s doesn't exist\n", CWD.c_str());
        CWD.set_length(len);
        return -1;
    }

    SdFile dir;
    dir.open(CWD.c_str(), O_READ);
    if (!dir.isDir())
    {
        SystemIO::printf("%s is not a dir\n", CWD.c_str());
        CWD.set_length(len);
        return -1;
    }

    dir.close();

    return 0;
}

int listdir(int argc, const char* argv[])
{
    SdFile dir;

    bool print_size = false;

    for (int i = 0; i < argc; i++)
    {
        if (string_equals(argv[0], "-psize"))
            print_size = true;
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
            SystemIO::flush();
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
        SystemIO::printf("error: no file was specified, exiting...\n");
        return -1;
    }

    bool print_register_dump = false;
    for (int i = 1; i < argc; i++)
    {
        if (string_equals(argv[i], "-rdump"))
            print_register_dump = true;
    }

    SystemIO::printf("Loading program...\n");

    TypeMacros::u32 len = CWD.length();

    CWD.append(argv[0]);

    SDCardFileStream file(CWD.c_str(), READ);
    bool opened = SystemIO::open_file(&file);
    if (!opened)
    {
        CWD.set_length(len);
        SystemIO::printf("error: failed to open file, exiting...\n");
        return -1;
    }

    CWD.set_length(len);

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

    bool success = VM.prepare(
        &outputStream,
        &inputStream,
        &basicAllocator,
        Program,
        appendIndex,
        STARTING_HEAP_SIZE
    );

    if (!success)
    {
        SystemIO::printf("Memory Allocation errors, failed to allocate program memory\n");
        return -1;
    }

    int rs = VM.start_program();

    if ((rs != VMErrors::PASS && rs != VMErrors::HALTED) || print_register_dump)
        VM.register_dump();

    VM.cleanup();

    SystemIO::printf("Program exited with return code = %d\n", rs);

    return 0;
}

int print_file(int argc, const char* argv[])
{
    if (argc <= 0)
    {
        SystemIO::printf("error: no input file specified, exiting...\n");
        return -1;
    }

    TypeMacros::u32 len = CWD.length();

    CWD.append(argv[0]);

    SDCardFileStream stream(argv[0], READ);
    if (!SystemIO::open_file(&stream))
    {
        CWD.set_length(len);
        SystemIO::printf("error: failed to open file %s\n", argv[0]);
        return -1;
    }

    CWD.set_length(len);

    TypeMacros::u8 buffer[256];
    while (stream.available() > 0)
    {
        TypeMacros::u32 bytesRead = stream.read(buffer, 256);
        outputStream.write(buffer, bytesRead);
    }

    SystemIO::close_file(&stream);

    return 0;
}

int mkdir(int argc, const char* argv[])
{
    if (argc <= 0)
    {
        SystemIO::printf("error: no input directory name specified, exiting...\n");
        return -1;
    }

    bool pFlag = false;

    for (int i = 1; i < argc; i++)
    {
        if (string_equals(argv[i], "-rooted"))
            pFlag = true;
    }

    TypeMacros::u32 len = CWD.length();

    CWD.append(argv[0]);
    if (!SDCardFileStream::card.mkdir(CWD.c_str(), pFlag))
    {
        SystemIO::printf("error: failed to create directory %s\n", CWD.c_str());
        CWD.set_length(len);
        return -1;
    }

    CWD.set_length(len);

    return 0;
}

int rmdir(int argc, const char* argv[])
{
    if (argc <= 0)
    {
        SystemIO::printf("error: no input directory name specified, exiting...\n");
        return -1;
    }

    bool pFlag = false;

    for (int i = 1; i < argc; i++)
    {
        if (string_equals(argv[i], "-rooted"))
            pFlag = true;
    }

    TypeMacros::u32 len = CWD.length();

    CWD.append(argv[0]);
    if (!SDCardFileStream::card.rmdir(CWD.c_str()))
    {
        SystemIO::printf("error: failed to delete the directory %s\n", CWD.c_str());
        CWD.set_length(len);
        return -1;
    }

    CWD.set_length(len);
    return 0;
}

int del(int argc, const char* argv[])  
{
    if (argc <= 0)
    {
        SystemIO::printf("error: no input filename specified, exiting...\n");
        return -1;
    }

    u32 len = CWD.length();

    CWD.append(argv[0]);

    if (!SDCardFileStream::card.remove(CWD.c_str()))
    {
        SystemIO::printf("error: unknown error occured while trying to remove file %s\n", CWD.c_str());
        CWD.set_length(len);
        return -1;
    }

    CWD.set_length(len);

    return 0;
}

int create(int argc, const char* argv[])
{
    if (argc <= 0)
    {
        SystemIO::printf("error: no input filename specified, exiting...\n");
        return -1;
    }

    u32 len = CWD.length();

    CWD.append(argv[0]);

    SdFile file;
    if (!file.open(CWD.c_str(), O_CREAT | O_WRITE))
    {
        SystemIO::printf("error: failed to create file %s\n", CWD.c_str());
        CWD.set_length(len);
        return -1;
    }

    file.close();
    CWD.set_length(len);

    return 0;
}

#endif

#ifdef INCLUDE_DEBUG_FUNCTIONS

int log_free_mem(int argc, const char* argv[])
{
    SystemIO::printf("Free Heap: %u bytes\n", esp_get_free_heap_size());
    SystemIO::printf("Minimum Free Heap: %u bytes\n", esp_get_minimum_free_heap_size());
    SystemIO::printf("Largest Free Block: %u bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));

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

    SystemIO::printf("%s version %s Initialization process done\n", OS_NAME, VERSION);
    SystemIO::printf("A single slash ('/') means that you are at the root directory\n");
    SystemIO::printf("$(/) <- ");
}

void loop()
{
    u32 bytesRead = SystemIO::gets(CLS, MAX_STATEMENT_LENGTH);
    if (bytesRead <= 0)
        return;

    SystemIO::printf("%s\n", CLS);

    CommandLine::FunctionPackage* function = CMD.process_statement(CLS);

    if (function != nullptr) {
        runCMDFunction(function);
    } else {
        SystemIO::printf("%s is not valid command\n", CLS);
    }

    SystemIO::printf("$(%s) <- ", CWD.c_str());
}

void loadEverything()
{
    // Load the default streams
    allFiles.push(&outputStream);
    allFiles.push(&inputStream);
    allFiles.push(&errorStream);

    SystemIO::printf("Initialized Standard I/O Streams\n");

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

    CMD.register_function("lsdir", listdir);
    CMD.register_function("start", run_program);
    CMD.register_function("dumpf", print_file);
    CMD.register_function("cd", change_directory);
    CMD.register_function("mkdir", mkdir);
    CMD.register_function("rmdir", rmdir);
    CMD.register_function("create", create);
    CMD.register_function("delete", del);

#endif

#ifdef INCLUDE_DEBUG_FUNCTIONS

    CMD.register_function("freemem", log_free_mem);

#endif

    CMD.register_function("restart", reset_machine);
    CMD.register_function("echo", echo);
}

void runCMDFunction(CommandLine::FunctionPackage* f)
{
    f->function(f->argc, f->argv);
}