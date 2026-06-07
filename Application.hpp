#ifndef __EMBEDI_APPLICATION_HPP__
#define __EMBEDI_APPLICATION_HPP__

#include "Allocator.hpp"
#include "TypeMacros.hpp"
#include "IOStreams.hpp"

struct Application
{   
    virtual int run(int argc, const char* argv[]) = 0;
    virtual void interrupt() = 0;
    virtual void signal(int sigint) = 0;
};

#endif