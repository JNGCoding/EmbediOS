#ifndef __EMBEDI_APPLICATION_HPP__
#define __EMBEDI_APPLICATION_HPP__

#include "Allocator.hpp"
#include "TypeMacros.hpp"
#include "IOStreams.hpp"

constexpr int CTRL_C_SIGNAL = 0x01;

struct EmbediApplication
{
    volatile bool interrupted = false;

    // Application main entry point, program will be launched from here
    // with command line arguments
    // `argc`: argument count
    // `argv`: argument vector
    // warning: All the interrupt functionality should be implemented within this function
    // to minimise overhead, the Application is not inserted with any event loop system
    // to automatically launch handlers upon any event.
    virtual int main(int argc, const char* argv[]) = 0;

    // this function is a special function which will flag the process as interrupted
    // upon which the program will launch its interrupt handler which might close it
    // or handle it.
    virtual void interrupt()
    {
        this->interrupted = true;
    }

    // this function is a handler function for various signals provided by the user
    // for eg. 'ctrl+c' will by default close the program
    virtual void signal(int sigint)
    {
        switch (sigint)
        {
            case CTRL_C_SIGNAL:
                this->interrupt();
                break;
        }
    }
};

#endif