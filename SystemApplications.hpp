#include "FreeRTOSConfig.h"
#ifndef __EMBEDI_SYSTEM_APPLICATIONS_HPP__
#define __EMBEDI_SYSTEM_APPLICATIONS_HPP__

#include "Application.hpp"
#include "TypeMacros.hpp"
#include "IOStreams.hpp"
#include "Allocator.hpp"
#include "DataStructures.hpp"

// IDK, I don't have a keyboard so Don't know how to do this
// Thinking of making vim ???? like the BadTextEditor will have multiple states
// in append mode, you can append text
// in insert mode, <line number> <text>
// in command mode, you can use various commands
// but since I don't have a keyboard to attach to ESP32
// also the ESP32 is not capable of USB reading so this is kind off hard
// but also, If I attach a keyboard, then I would have to create interrupts and create
// my own buffered input stream to read characters typed
struct BadTextEditor : public EmbediApplication
{
    enum class Mode
    {
        APPEND,
        INSERT,
        COMMAND
    };

    Mode mode = Mode::COMMAND;
    EmbediString fileContent;
    SDCardFileStream stream;

    BadTextEditor(const char* path, EmbediAllocator* allocator) : fileContent("", allocator), stream(path, READ)
    {}

    ~BadTextEditor()
    {
        SystemIO::close_file(&this->stream);
    }

    int main(int argc, const char* argv[])
    {
        if (!SystemIO::open_file(&this->stream))
        {
            SystemIO::perror("error: failed to open file\n");
            return -1;
        }

        {
            TypeMacros::u8 buffer[256];
            while (this->stream.available() > 0)
            {
                TypeMacros::u32 bytesRead = this->stream.read(buffer, 255);
                buffer[bytesRead] = 0;
                this->fileContent.append(reinterpret_cast<const char*>(buffer));
            }
        }

        SystemIO::printf("Bad text editor v0.0\n");
        SystemIO::printf("File content:\n%s\n", this->fileContent);

        return 0;
    }
};

#endif