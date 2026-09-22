#ifndef __OS_SETUP_HPP__
#define __OS_SETUP_HPP__

#include <Arduino.h>
#include <SdFat.h>

#include "TypeMacros.hpp"
#include "Defines.hpp"

#include "FileSystem/IOStreams.hpp"
#include "CommandLine/CommandLine.hpp"

#include "VirtualMachine/VirtualMachine.hpp"

#include "OperatingSystem.hpp"

using namespace TypeMacros;

#define MACHINE_RESET_FUNC() ESP.restart()
#define CMD_FUNC(func) int func(int argc, const char* argv[])

CMD_FUNC(echo);
CMD_FUNC(restart_machine);
CMD_FUNC(create_var);
CMD_FUNC(change_var);
CMD_FUNC(delete_var);
CMD_FUNC(pin_mode_set);
CMD_FUNC(pin_out_set);
CMD_FUNC(print_pin_read);

#ifdef SUPPORTS_PWM

CMD_FUNC(pwm_pin_set);
CMD_FUNC(pwm_set);

#endif

#ifdef SUPPORTS_ADC

CMD_FUNC(adc_pin_set);
CMD_FUNC(print_adc_pin_read);

#endif

#ifdef SUPPORTS_DAC

CMD_FUNC(dac_pin_set);
CMD_FUNC(dac_set);

#endif

#ifdef COMPILE_WITH_SD_INITIALIZATION

CMD_FUNC(change_directory);
CMD_FUNC(listdir);
CMD_FUNC(run_program);
CMD_FUNC(print_file);
CMD_FUNC(mkdir);
CMD_FUNC(rmdir);
CMD_FUNC(del);
CMD_FUNC(create);

#endif

#ifdef INCLUDE_DEBUG_FUNCTIONS

CMD_FUNC(log_free_mem);

#endif

#ifdef COMPILE_WITH_SD_INITIALIZATION

SdFat SDCardFileStream::card; // fs

#endif

extern OperatingSystem OS;

// This function has to be attached to an interrupt to stop an vm program
// from running upon the press of a button or anyway of how the programmer would like
// to trigger this interrupt
void ARDUINO_ISR_ATTR interrupt_vm()
{
    OS.VM.ERROR_CODE = VMErrors::VM_INTERRUPTED;
}

// Load the stdout, stdin, stderr in the file table
bool setup_streams(EmbediFileStream* out, EmbediFileStream* inp, EmbediFileStream* err)
{
    OS.outputStream = out;
    OS.inputStream = inp;
    OS.errorStream = err;

    if (out != nullptr) OS.fileTable.push(out);
    if (inp != nullptr) OS.fileTable.push(inp);
    if (err != nullptr) OS.fileTable.push(err);

    return (out != nullptr) && (inp != nullptr) && (err != nullptr);
}

// Load basic functions into the Command line
// echo, restart_machine
void load_starter_functions()
{
    OS.terminal.register_function("restart", restart_machine);
    OS.terminal.register_function("echo", echo);
    OS.terminal.register_function("defvar", create_var);
    OS.terminal.register_function("redefvar", change_var);
    OS.terminal.register_function("delvar", delete_var);
}

// Load display functions

// Load all basic GPIO Control functions into the Command Line
// pinmode_set, pinout_set, pinread, .... etc. Look the manual for more info
void load_gpio_functions()
{
    OS.terminal.register_function("pinmode_set", pin_mode_set);
    OS.terminal.register_function("pinout_set", pin_out_set);
    OS.terminal.register_function("pinread", print_pin_read);

#ifdef SUPPORTS_PWM

    OS.terminal.register_function("pwmpin_set", pwm_pin_set);
    OS.terminal.register_function("pwmwrite", pwm_set);

#endif

#ifdef SUPPORTS_ADC

    OS.terminal.register_function("adcpin_set", adc_pin_set);
    OS.terminal.register_function("adcread", print_adc_pin_read);

#endif

#ifdef SUPPORTS_DAC

    OS.terminal.register_function("dacpin_set", dac_pin_set);
    OS.terminal.register_function("dacwrite", dac_set);

#endif
}

// Load basic debug or utility functions into the Command Line
// freemem
void load_debug_functions()
{
#ifdef INCLUDE_DEBUG_FUNCTIONS
    OS.terminal.register_function("freemem", log_free_mem);
#endif
}

// Load inbuilt SD Card Controller functions into the Command Line
// cd, lsdir, dumpf, .... etc. Look the manual for more info
void load_sd_functions()
{
#ifdef COMPILE_WITH_SD_INITIALIZATION
    OS.terminal.register_function("lsdir", listdir);
    OS.terminal.register_function("start", run_program);
    OS.terminal.register_function("dumpf", print_file);
    OS.terminal.register_function("cd", change_directory);
    OS.terminal.register_function("mkdir", mkdir);
    OS.terminal.register_function("rmdir", rmdir);
    OS.terminal.register_function("create", create);
    OS.terminal.register_function("delete", del);
#endif
}

// Setup and Initialize SD Card Controller
bool setup_sd_card()
{
#ifdef COMPILE_WITH_SD_INITIALIZATION
    // Load the SdFat variables to intialize SdCard Streams
    return SDCardFileStream::card.begin(SD_CARD_CHIP_SELECT, SD_SCK_MHZ(8));
#else
    return false;
#endif
}

// Setup the VM interrupt and attach it to `PROGRAM_INTERRUPT_PIN`
void setup_vm_interrupt()
{
    if (PROGRAM_INTERRUPT_PIN >= 0)
    {
        pinMode(PROGRAM_INTERRUPT_PIN, INTERRUPT_INPUT_MODE);
        attachInterrupt(digitalPinToInterrupt(PROGRAM_INTERRUPT_PIN), interrupt_vm, INTERRUPT_PIN_CHANGE_STYLE);
    }
}

// should be called in loop(), scans the stdin for input and gives to Command Line
// for processing
void run_basic_loop_iteration()
{
    char statement[MAX_STATEMENT_LENGTH] = {0};

    u32 bytesRead = SystemIO::gets(statement, MAX_STATEMENT_LENGTH);
    if (bytesRead <= 0)
        return;

    SystemIO::printf("%s\n", statement);

    CommandLine::FunctionPackage* f = OS.terminal.process_statement(statement);

    if (f != nullptr)
    {
        unsigned long start_time = millis();
        f->function(f->argc, f->argv);
        unsigned long end_time = millis();
        SystemIO::printf("Execution completed in %ums\n", static_cast<uint>(end_time - start_time));
    }
    else
    {
        SystemIO::printf("\"%s\" is not valid command\n", statement);
    }

    SystemIO::printf("$(%s) <- ", OS.cwd.c_str());
}

// Gives the provided statement to the Command Line for processing
void run_loop_iteration_with_line(const char* statement)
{
    CommandLine::FunctionPackage* f = OS.terminal.process_statement(statement);

    if (f != nullptr)
    {
        unsigned long start_time = millis();
        f->function(f->argc, f->argv);
        unsigned long end_time = millis();
        SystemIO::printf("Execution completed in %ums\n", static_cast<uint>(end_time - start_time));
    }
    else
    {
        SystemIO::printf("\"%s\" is not valid command\n", statement);
    }

    SystemIO::printf("$(%s) <- ", OS.cwd.c_str());
}

// Only works if COMPILE_WITH_SD_INITIALIZATION is defined
// Searchs for a file called `os_init/init.cmd` in the SD Card
// and runs all the CMD commands directly after bootup
void run_bootafter_file()
{
#ifdef COMPILE_WITH_SD_INITIALIZATION
    if (SDCardFileStream::card.exists("/os_init/init.cmd"))
    {
        SDCardFileStream file("/os_init/init.cmd", READ);
        char Line[MAX_STATEMENT_LENGTH] = {0};
        
        if (!SystemIO::open_file(&file))
        {
            SystemIO::perror("failed to read afterboot init file\n");
        }
        else
        {
            u32 appendIndex = 0;
            while (SystemIO::favailable(&file) > 0)
            {
                char c = SystemIO::fgetc(&file);
                if (c != '\n')
                {
                    Line[appendIndex++] = c;
                }
                else
                {
                    CommandLine::FunctionPackage* f = OS.terminal.process_statement(Line);
                    if (f != nullptr)
                    {
                        f->function(f->argc, f->argv);
                    }
                    else
                    {
                        SystemIO::fprintf(SYSERR_NAME, "init.cmd: \"%s\" is not valid command\n", Line);
                    }

                    memset(Line, 0, MAX_STATEMENT_LENGTH);
                    appendIndex = 0;
                }
            }

            // process any residual command
            CommandLine::FunctionPackage* f = OS.terminal.process_statement(Line);
            if (f != nullptr) { f->function(f->argc, f->argv); }
            else
                SystemIO::fprintf(SYSERR_NAME, "init.cmd: %s is not valid command\n", Line);

            SystemIO::close_file(&file);
        }
    }
    else
        SystemIO::perror("init.cmd file not found\n");
#endif
}

#ifdef COMPILE_WITH_SD_INITIALIZATION

int change_directory(int argc, const char* argv[])
{
    if (argc <= 0)
    {
        SystemIO::printf("error: no input directory specified, exiting...\n");
        return -1;
    }

    u32 len = OS.cwd.length();

    if (string_equals(argv[0], "/"))
    {
        OS.cwd.set("/");
        return 0;
    }

    if (string_equals(argv[0], ".."))
    {
        if (string_equals(OS.cwd.c_str(), "/"))
            return 0;

        for (i32 i = static_cast<i32>(len) - 2; i >= 0; i--)
        {
            if (OS.cwd.get_char(i) == '/')
                OS.cwd.set_length(i + 1);  // Remove the '/'
        }

        return 0;
    }

    OS.cwd.append(argv[0]);
    if (OS.cwd.get_char(OS.cwd.length() - 1) != '/')
        OS.cwd.append("/");

    if (!SDCardFileStream::exists(OS.cwd.c_str()))
    {
        SystemIO::printf("Path %s doesn't exist\n", OS.cwd.c_str());
        OS.cwd.set_length(len);
        return -1;
    }

    SdFile dir;
    dir.open(OS.cwd.c_str(), O_READ);
    if (!dir.isDir())
    {
        SystemIO::printf("%s is not a dir\n", OS.cwd.c_str());
        OS.cwd.set_length(len);
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

    const char* path = OS.cwd.c_str();
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
                usize bytes = 0, bytesAvailable = 0;
                while ((bytesAvailable = file.available()) > 0)
                {
                    usize incrementer = embedi_min(24ULL, bytesAvailable);
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
    if (argc <= 0)
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

    u32 len = OS.cwd.length();

    OS.cwd.append(argv[0]);

    SDCardFileStream file(OS.cwd.c_str(), READ);
    bool opened = SystemIO::open_file(&file);
    if (!opened)
    {
        OS.cwd.set_length(len);
        SystemIO::printf("error: failed to open file, exiting...\n");
        return -1;
    }

    OS.cwd.set_length(len);

    u32 appendIndex = 0;
    while (SystemIO::favailable(&file) > 0)
    {
        OS.programMem[appendIndex++] = file.read();
        if (appendIndex >= MAX_PROGRAM_SIZE)
        {
            SystemIO::printf("error: program size is greater than size limit: %u bytes, exiting...\n", MAX_PROGRAM_SIZE);
            return -1;
        }
    }

    SystemIO::close_file(&file);

    SystemIO::printf("Program successfully read, program size: %u bytes\n", appendIndex);

    bool success = OS.VM.prepare(
        OS.fileTable.get_handle(SYSOUT_NAME),
        OS.fileTable.get_handle(SYSIN_NAME),
        OS.fileTable.get_handle(SYSERR_NAME),
        &basicAllocator,
        OS.programMem,
        appendIndex,
        STARTING_HEAP_SIZE
    );

    if (!success)
    {
        SystemIO::printf("Memory Allocation errors, failed to allocate program memory\n");
        return -1;
    }

    int rs = OS.VM.start_program();

    if (rs == VMErrors::VM_INTERRUPTED)
        SystemIO::printf("Program interrupted with interrupt button\n");

    if ((rs != VMErrors::PASS && rs != VMErrors::HALTED && rs != VMErrors::VM_INTERRUPTED) || print_register_dump)
        OS.VM.register_dump();

    OS.VM.cleanup();

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

    bool printLineNumbers = false;
    for (int i = 1; i < argc; i++)
    {
        if (string_equals(argv[i], "-printlnum"))
            printLineNumbers = true;
    }

    u32 len = OS.cwd.length();

    OS.cwd.append(argv[0]);

    SDCardFileStream stream(OS.cwd.c_str(), READ);
    if (!SystemIO::open_file(&stream))
    {
        OS.cwd.set_length(len);
        SystemIO::printf("error: failed to open file %s\n", argv[0]);
        return -1;
    }

    OS.cwd.set_length(len);

    u8 buffer[256];

    unsigned int lineNumber = 1;
    SystemIO::printf("%u ", lineNumber++);

    if (printLineNumbers)
    {
        char c = stream.read();
        if (c == '\n')
        {
            SystemIO::printf("\n%u ", lineNumber++);
        }
        else SystemIO::printchar(c);
    }
    else
    {
        while (SystemIO::favailable(&stream) > 0)
        {
            u32 bytesRead = stream.read(buffer, 255);
            buffer[bytesRead] = 0;
            SystemIO::printf(reinterpret_cast<const char*>(buffer));
        }
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

    u32 len = OS.cwd.length();

    OS.cwd.append(argv[0]);
    if (!SDCardFileStream::mkdir(OS.cwd.c_str(), pFlag))
    {
        SystemIO::printf("error: failed to create directory %s\n", OS.cwd.c_str());
        OS.cwd.set_length(len);
        return -1;
    }

    OS.cwd.set_length(len);

    return 0;
}
int rmdir(int argc, const char* argv[])
{
    if (argc <= 0)
    {
        SystemIO::printf("error: no input directory name specified, exiting...\n");
        return -1;
    }

    u32 len = OS.cwd.length();

    OS.cwd.append(argv[0]);
    if (!SDCardFileStream::rmdir(OS.cwd.c_str()))
    {
        SystemIO::printf("error: failed to delete the directory %s\n", OS.cwd.c_str());
        OS.cwd.set_length(len);
        return -1;
    }

    OS.cwd.set_length(len);
    return 0;
}
int del(int argc, const char* argv[])  
{
    if (argc <= 0)
    {
        SystemIO::printf("error: no input filename specified, exiting...\n");
        return -1;
    }

    u32 len = OS.cwd.length();

    OS.cwd.append(argv[0]);

    if (!SDCardFileStream::remove(OS.cwd.c_str()))
    {
        SystemIO::printf("error: unknown error occured while trying to remove file %s\n", OS.cwd.c_str());
        OS.cwd.set_length(len);
        return -1;
    }

    OS.cwd.set_length(len);

    return 0;
}
int create(int argc, const char* argv[])
{
    if (argc <= 0)
    {
        SystemIO::printf("error: no input filename specified, exiting...\n");
        return -1;
    }

    u32 len = OS.cwd.length();

    OS.cwd.append(argv[0]);

    if (!SDCardFileStream::create(OS.cwd.c_str()))
    {
        SystemIO::printf("error: failed to create file %s\n", OS.cwd.c_str());
        OS.cwd.set_length(len);
        return -1;
    }

    OS.cwd.set_length(len);

    return 0;
}

#endif

// Some basic GPIO Functions
int pin_mode_set(int argc, const char* argv[])
{
    if (argc < 2)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    u8 pin = static_cast<u8>(atoi(argv[0]));

    if (string_equals(argv[1], "OUTPUT")) {
        pinMode(pin, OUTPUT);
    }
    else if (string_equals(argv[1], "INPUT")) {
        pinMode(pin, INPUT);
    }
    else if (string_equals(argv[1], "INPUT_PULLUP")) {
        pinMode(pin, INPUT_PULLUP);
    }
    else if (string_equals(argv[1], "INPUT_PULLDOWN")) {
        pinMode(pin, INPUT_PULLDOWN);
    }
    else {
        SystemIO::printf("error: invalid mode given %s\n", argv[1]);
        return -1;
    }

    return 0;
}

int pin_out_set(int argc, const char* argv[])
{
    if (argc < 2)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    u8 pin = static_cast<u8>(atoi(argv[0]));

    if (string_equals(argv[1], "HIGH")) {
        digitalWrite(pin, HIGH);
    }
    else if (string_equals(argv[1], "LOW")) {
        digitalWrite(pin, LOW);
    }
    else {
        SystemIO::printf("error: invalid mode given %s\n", argv[1]);
        return -1;
    }

    return 0;
}

int print_pin_read(int argc, const char* argv[])
{
    if (argc < 1)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    u8 pin = static_cast<u8>(atoi(argv[0]));
    SystemIO::printf("Pin %bu read %d", pin, digitalRead(pin));

    return 0;
}

#ifdef SUPPORTS_PWM

int pwm_pin_set(int argc, const char* argv[])
{
    if (argc < 1)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    u8 pin = static_cast<u8>(atoi(argv[0]));
    pinMode(pin, OUTPUT);

    return 0;
}

int pwm_set(int argc, const char* argv[])
{
    if (argc < 2)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    u8 pin = static_cast<u8>(atoi(argv[0]));
    u8 inp = static_cast<u8>(atoi(argv[1]));

    analogWrite(pin, inp);

    return 0;
}

#endif

#ifdef SUPPORTS_ADC

int adc_pin_set(int argc, const char* argv[])
{
    if (argc < 1)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    u8 pin = static_cast<u8>(atoi(argv[0]));
    pinMode(pin, INPUT);

    return 0;
}

int print_adc_pin_read(int argc, const char* argv[])
{
    if (argc < 1)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    u8 pin = static_cast<u8>(atoi(argv[0]));
    SystemIO::printf("ADC Pin %bu read analog value %u", pin, analogRead(pin));

    return 0;
}

#endif

#ifdef SUPPORTS_DAC

int dac_pin_set(int argc, const char* argv[])
{
    if (argc < 1)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    u8 pin = static_cast<u8>(atoi(argv[0]));
    pinMode(pin, OUTPUT);

    return 0;
}

int dac_set(int argc, const char* argv[])
{
    if (argc < 2)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    u8 pin = static_cast<u8>(atoi(argv[0]));
    u8 dac = static_cast<u8>(atoi(argv[1]));
    dacWrite(pin, dac);

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


int restart_machine(int argc, const char* argv[])
{
    MACHINE_RESET_FUNC();
    return 0;
}

int echo(int argc, const char* argv[])
{
    for (int i = 0; i < argc; i++)
        SystemIO::printf("%s\n", argv[i]);

    SystemIO::flush();
    
    return 0;
}

int create_var(int argc, const char* argv[])
{
    if (argc == 0)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    OS.systemVariables.create(argv[0]);

    if (argc == 2)
        OS.systemVariables.change(argv[0], argv[1]);

    return 0;
}

int change_var(int argc, const char* argv[])
{
    if (argc <= 2)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    OS.systemVariables.change(argv[0], argv[1]);

    return 0;
}

int delete_var(int argc, const char* argv[])
{
    if (argc <= 1)
    {
        SystemIO::printf("error: insufficient arguments specified\n");
        return -1;
    }

    OS.systemVariables.remove(argv[0]);

    return 0;
}

#undef MACHINE_RESET_FUNC
#undef CMD_FUNC

#endif