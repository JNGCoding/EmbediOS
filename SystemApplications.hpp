#ifndef __EMBEDI_SYSTEM_APPLICATIONS_HPP__
#define __EMBEDI_SYSTEM_APPLICATIONS_HPP__

#include "Application.hpp"

struct Demo : public EmbediApplication
{
    int main(int argc, const char* argv[]) override;
};

extern Demo demoApp;

#endif