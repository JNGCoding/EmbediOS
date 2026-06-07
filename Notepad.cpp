#include "SystemApplications.hpp"

int HelloWorld::run(int argc, const char* argv[])
{
    SystemIO::printf("Hello, World\n");
    return 0;
}

void HelloWorld::interrupt()
{
}

void HelloWorld::signal(int sigint)
{
}