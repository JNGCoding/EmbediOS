#include "SystemApplications.hpp"

int Demo::main(int argc, const char* argv[])
{
    SystemIO::printf("Hello, World! Application started.\n");
    SystemIO::printf("Arguments (%d):\n", argc);

    for (int i = 0; i < argc; ++i)
        SystemIO::printf("  argv[%d] = %s\n", i, argv[i]);

    if (!interrupted)
    {
        SystemIO::printf("Running normally...\n");
    }
    else SystemIO::printf("Application was interrupted!\n");

    return 0;
}

Demo demoApp;