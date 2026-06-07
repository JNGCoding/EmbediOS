#ifndef __EMBEDI_SYSTEM_APPLICATIONS_HPP__
#define __EMBEDI_SYSTEM_APPLICATIONS_HPP__

#include "Application.hpp"

struct HelloWorld : public Application
{
    int run(int argc, const char* argv[]) override;
    void interrupt() override;
    void signal(int sigint) override;
};

#endif