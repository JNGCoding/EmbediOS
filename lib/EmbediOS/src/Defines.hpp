#ifndef __EMBEDI_DEFINES_HPP__
#define __EMBEDI_DEFINES_HPP__

#include <Arduino.h>
#include "TypeMacros.hpp"

// AUTHOR'S NOTE
// FOR DIFFERENT HARDWARE, YOU MAY HAVE TO IMPLEMENT DIFFERENT DRIVERS
// FOR THAT, YOU MUST EDIT THE RESPECTIVE DRIVER FILES
// SUCH AS, FOR WIRELESS CONNECTIVITY - WIFI, BLUETOOTH
// YOU MAY HAVE TO EDIT WiFiManager.cpp and BluetoothManager.cpp
// ADDITIONALLY, IF YOU DON'T NEED THOSE FUNCTIONALITY
// YOU CAN REMOVE THOSE FILES FROM THE COMPILATION PROCESS

constexpr const char* VARIANT_NAME = "DevSetup";
constexpr const char* VERSION = "0.00B";

// MAX COMMAND LINE STATEMENT LENGTH
constexpr TypeMacros::u32 MAX_STATEMENT_LENGTH = Kb(1);

// NUMBER OF ARGUMENTS AT MAX PASSABLE TO AN APPLICATION
constexpr TypeMacros::u32 MAX_ARGS = 5;

// MAX LENGTH OF ONE ARBITRARY ARGUMENT PASSABLE
// IF ANY ARGUMENT GREATER THAN THIS VALUE IS PASSED
// THEN THE ARGUMENT WILL BE TRUNCATED
constexpr TypeMacros::u32 ARG_LENGTH = 64;

// MAX NUMBER OF FUNCTIONS THAT THE CMD CAN STORE IN ITS BUFFERS
constexpr TypeMacros::u32 MAX_FUNCTIONS = 128;

// SYSTEM VARIABLE NAME MAX LENGTH
constexpr TypeMacros::u32 MAX_SYSTEM_VAR_NAME_LENGTH = 128;

// SCRAPPED THIS IDEA, NOW SYSTEM_VARIABLE_VALUE == ARG_LENGTH
// constexpr TypeMacros::u32 MAX_SYSTEM_VARIABLE_LENGTH = 128;

// SYSTEM VARIABLE MEMORY SPACE
constexpr TypeMacros::u32 SYSTEM_VARIABLES_MEMORY = Kb(10);

// MAX PROGRAM SIZE
constexpr TypeMacros::u32 MAX_PROGRAM_SIZE = Kb(10);

// PROGRAM STACK SIZE
constexpr TypeMacros::u32 STACK_SIZE = Kb(1);

// SD CARD SPI CS SELECT PIN
constexpr TypeMacros::u8 SD_CARD_CHIP_SELECT = 5;

// OPERATIONAL MEMORY SIZE
constexpr TypeMacros::u32 OPERATIONAL_MEMORY_SIZE = Kb(1);

// PROGRAM DEFAULT HEAP SIZE AT START
constexpr TypeMacros::u32 STARTING_HEAP_SIZE = 512;

// I2C (WIRE) CLOCK SPEED
constexpr TypeMacros::u32 WIRE_CLOCK_SPEED = 800000UL;

// PROGRAM INTERRUPT PIN - USED FOR INTERRUPTING A PROGRAM
// INTERRUPT PIN IS NOT SET IF THIS SPEC IS SET TO ANY VALUE LESS THAN 0 ( < 0 )
constexpr TypeMacros::u8 PROGRAM_INTERRUPT_PIN = 0;
constexpr TypeMacros::u8 INTERRUPT_INPUT_MODE = INPUT;
constexpr TypeMacros::u8 INTERRUPT_PIN_CHANGE_STYLE = RISING;

// Initializes Display
// #define COMPILE_WITH_DISPLAY_INITIALIZATION
#ifndef COMPILE_WITH_DISPLAY_INITIALIZATION
#warning Compiling without Display Drivers, Functions removed
#endif

// SD CARD INITIALIZATION
// #define COMPILE_WITH_SD_INITIALIZATION
#ifndef COMPILE_WITH_SD_INITIALIZATION
#warning Compiling without SDCard initializations, Functions removed: lsdir, cd, start, dumpf, mkdir, rmdir, create, delete
#endif

// Doesn't work for now
// #define COMPILE_WITH_WIFI_INITIALIZATION
#ifndef COMPILE_WITH_WIFI_INITIALIZATION
#warning Compiling without WiFi initializations
#endif

// #define COMPILE_WITH_BLUETOOTH_INITIALIZATION
#ifndef COMPILE_WITH_BLUETOOTH_INITIALIZATION
#warning Compiling without Bluetooth initializations
#endif

// Basic stuff that every micro-controller should include with itself
// but some of the micro-controllers don't because they are specialized for some task
// for eg. some of the PIC micro-controllers don't even have PWM and ADCs
// but I don't think this software will be compiled for them but any way
// I am providing an option for turning off some very basic settings
// Also they have been given special name starting with SUPPORTS because there is no dedicated
// driver for them, These things are already generalized by the Arduino Framework
#define SUPPORTS_PWM
#define SUPPORTS_ADC
// #define SUPPORTS_DAC

// DEBUG FUNCTIONS
// #define INCLUDE_DEBUG_FUNCTIONS
#ifndef INCLUDE_DEBUG_FUNCTIONS
#warning Compiling without debug functions, Functions removed: freemem
#endif

// VM OPTIONS

// Enable it to ensure identical time consumed in between executing instructions
// For eg. a noop instruction might be faster than mov instruction
// so a noop instruction might exit quickly than a mov instruction
// This options prevents that by targeting a "INSTRUCTIONS_PER_SECOND" speed
// via delays(). Since the VM stays idle in that time, could easily be paired
// with an RTOS to make the delay concurrent with other tasks
#define MAINTAIN_STABLE_CLOCK

#ifdef MAINTAIN_STABLE_CLOCK

// The Actual IPS of VM may jitter due to timing overhead, In my setup
// where I am using an Standard DOIT ESP32 Wroom Devkit to reach the target IPS
// I am using target IPS + 20K since the batch based timing overhead in a tight loop
// drops roughly 15k - 20k throughput of the VM

// This VM sucks, don't use in production unless you come up with good timing
// strategy, with the time drift and overhead, the VM may mostly undershoot the target
// as I am experiencing

// and I don't know how to make timing logic, and timing logic may vary between machines

// TARGET IPS
constexpr TypeMacros::u32 INSTRUCTIONS_PER_SECOND = 120000;

// EXPECTED TIME FOR EXECUTING ONE INSTRUCTION
constexpr TypeMacros::u32 TIME_FOR_ONE_INSTRUCTION = 1e6 / INSTRUCTIONS_PER_SECOND;

// NUMBER OF INSTRUCTIONS EXECUTED BEFORE FIRING THE PACING LOGIC
constexpr TypeMacros::u32 BATCH_SIZE = 100;

// Since I am using ESP32
// the delay and delayMicroseconds itself also add a lot of overhead
// in the VM's tight loop which is bad especially when the delayMicroseconds have to go through
// Arduino's layer and then FreeRTOS scheduler.

// You can define you own stalling methods to efficiently stall for more accurate stalls
// In my test runs I haven't been able to accurately get an IPS of 100k and undershoot to 85k
// because of the overhead of stalling I presume

#define VM_DELAY_US(x) delayMicroseconds((x))

#endif

constexpr TypeMacros::u32 EVENT_LOOP_QUEUE_SIZE = Kb(4);

#endif