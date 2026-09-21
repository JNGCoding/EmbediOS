#ifndef __EMBEDI_OS_OPERATING_SYSTEM_HPP__
#define __EMBEDI_OS_OPERATING_SYSTEM_HPP__

#include "TypeMacros.hpp"
#include "Defines.hpp"

#include "FileSystem/IOStreams.hpp"
#include "CommandLine/CommandLine.hpp"
#include "DataStructures/DataStructures.hpp"

#include "VirtualMachine/VirtualMachine.hpp"

void ARDUINO_ISR_ATTR interrupt_vm()
{ OS.__int_vm__(); }

class OperatingSystem
{
private:
    EmbediFileStream* inputStream  = nullptr;
    EmbediFileStream* outputStream = nullptr;
    EmbediFileStream* errorStream  = nullptr;

    EmbediString cwd;

    FileTable allFiles;

    CommandLine terminal;

    volatile VirtualMachine VM;
    u8 Program[MAX_PROGRAM_SIZE] = {0};

    void restart_machine()
    { ESP.restart(); }

    int echo(int argc, const char* argv[]);
    int restart_machine(int argc, const char* argv[]);
    int create_var(int argc, const char* argv[]);
    int change_var(int argc, const char* argv[]);
    int delete_var(int argc, const char* argv[]);
    int pin_mode_set(int argc, const char* argv[]);
    int pin_out_set(int argc, const char* argv[]);
    int print_pin_read(int argc, const char* argv[]);
    
#ifdef SUPPORTS_PWM
    
    int pwm_pin_set(int argc, const char* argv[]);
    int pwm_set(int argc, const char* argv[]);

#endif

#ifdef SUPPORTS_ADC

    int adc_pin_set(int argc, const char* argv[]);
    int print_adc_pin_read(int argc, const char* argv[]);

#endif

#ifdef SUPPORTS_DAC

    int dac_pin_set(int argc, const char* argv[]);
    int dac_set(int argc, const char* argv[]);

#endif

#ifdef COMPILE_WITH_SD_INITIALIZATION

    int change_directory(int argc, const char* argv[]);
    int list_directory(int argc, const char* argv[]);
    int run_program(int argc, const char* argv[]);
    int print_file(int argc, const char* argv[]);
    int make_directory(int argc, const char* argv[]);
    int remove_directory(int argc, const char* argv[]);
    int delete_file(int argc, const char* argv[]);
    int create_file(int argc, const char* argv[]);

#endif

#ifdef INCLUDE_DEBUG_FUNCTIONS

    int log_free_mem(int argc, const char* argv[]);

#endif

public:
    OperatingSystem() : allFiles(), terminal(), cwd("/")
    {}

    void __int_vm__()
    { this->VM.ERROR_CODE = VMErrors::VM_INTERRUPTED; }

    bool setup_streams(EmbediFileStream* outfs, EmbediFileStream* inpfs, EmbediFileStream* errfs)
    {
        this->outputStream = outfs;
        this->inputStream = inpfs;
        this->errorStream = errfs;

        if (outfs != nullptr) this->allFiles.push(outfs);
        if (inpfs != nullptr) this->allFiles.push(inpfs);
        if (errfs != nullptr) this->allFiles.push(errfs);

        return outfs != nullptr && inpfs != nullptr && errfs != nullptr;
    }

    bool setup_sd()
    {
#ifdef COMPILE_WITH_SD_INITIALIZATION
        // Load the SdFat variables to intialize SdCard Streams
        return SDCardFileStream::card.begin(SD_CARD_CHIP_SELECT, SD_SCK_MHZ(8));
#else
        return false;
#endif
    }

    bool setup_vm_interrupt()
    {
        if (PROGRAM_INTERRUPT_PIN >= 0)
        {
            pinMode(PROGRAM_INTERRUPT_PIN, INTERRUPT_INPUT_MODE);
            attachInterrupt(digitalPinToInterrupt(PROGRAM_INTERRUPT_PIN), interrupt_vm, INTERRUPT_PIN_CHANGE_STYLE);
        }

        return true;
    }

    void load_basic_functions()
    {
        this->terminal.register_function("restart", this->restart_machine);
        this->terminal.register_function("echo", this->echo);
    }

    void load_sd_functions()
    {
    }

    void load_gpio_functions()
    {
    }

    void load_debug_functions()
    {
    }
};

extern OperatingSystem OS;

#endif