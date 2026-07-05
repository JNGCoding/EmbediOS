#ifndef __EMBEDI_IO_STREAMS_HPP__
#define __EMBEDI_IO_STREAMS_HPP__

#include <Arduino.h>
#include <SdFat.h>
#include <string.h>
#include <GyverOLED.h>
#include <stdarg.h>
#include "TypeMacros.hpp"

#ifndef __AVR__
#include <assert.h>
#else
#define assert(expr) ((expr) ? (void)0 : abort())

// I fuckin hate AVR libc, and AVR libraries define getchar as a fuckin macro
// What the fuck, fucker gave me incomprehensible errors about redeclaration
// That's why an #undef is put here
#undef getchar

#endif

// At max how many files can be opened in the operating system simultaneously
//
// First 3 positions are reserved by stdout, stdin, syserr
constexpr TypeMacros::u32 MAX_FILE_HANDLES = 10;

// File Modes
constexpr TypeMacros::u8 READ   = 1 << 0;
constexpr TypeMacros::u8 WRITE  = 1 << 1;
constexpr TypeMacros::u8 APPEND = 1 << 2 | WRITE;

constexpr const char* SYSOUT_NAME = "stdout";
constexpr const char* SYSIN_NAME = "stdin";
constexpr const char* SYSERR_NAME = "stderr";

// This will link against the FileStream
struct FileDescriptor
{
    const char* name;
    const char* path;
    TypeMacros::u8 mode;
};

// An interface class for interfacing with file systems
struct EmbediFileStream
{
    FileDescriptor descriptor;
    
    virtual ~EmbediFileStream() = default;
    virtual bool open() { return false; }
    virtual void close() {}
    virtual bool write(const TypeMacros::u8 _byte) { return false; }
    virtual TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size) { return 0; }
    virtual void flush() {}
    virtual TypeMacros::u8 read() { return 0; }
    virtual TypeMacros::u32 read(TypeMacros::u8* buffer, TypeMacros::u32 size) { return 0; }
    virtual void seek(const TypeMacros::u32 pos) {}  // Relative to the start of the file
    virtual TypeMacros::u32 tell() { return 0; } // Relative to the start of the file
    virtual TypeMacros::u32 available() { return 0; }
};

struct FileTable
{
    EmbediFileStream* fileHandles[ MAX_FILE_HANDLES ];
    TypeMacros::u32 count = 0;

    EmbediFileStream* push(EmbediFileStream* fileStream);
    EmbediFileStream* remove(EmbediFileStream* fdesc);
    EmbediFileStream* remove(const int index);
    EmbediFileStream* get_handle(const int index);
    EmbediFileStream* get_handle(const char* streamName);
};

// This is the list that will be handled through-out the filespace
extern FileTable allFiles;

struct StandardOutput : public EmbediFileStream
{
    StandardOutput() { this->descriptor = {SYSOUT_NAME, "", WRITE}; }

    bool write(const TypeMacros::u8 _byte) override;
    TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size) override;
    void flush() override;
};

extern StandardOutput outputStream;

struct StandardInput : public EmbediFileStream
{
    StandardInput() { this->descriptor = {SYSIN_NAME, "", READ }; }
    
    TypeMacros::u8 read() override;
    TypeMacros::u32 read(TypeMacros::u8* buffer, TypeMacros::u32 size) override;
    TypeMacros::u32 available() override;
};

extern StandardInput inputStream;

// Using an oled display to display error logs
// To maximize control over components, it is encouraged to use composition instead
#undef OLED_WIDTH
template<int _TYPE, int _BUFF = OLED_BUFFER, int _CONN = OLED_I2C, int8_t _CS = -1, int8_t _DC = -1, int8_t _RST = -1>
class OLEDTerminal
{
private:
    GyverOLED<_TYPE, _BUFF, _CONN, _CS, _DC, _RST>& display;
    unsigned int lineIndex = 0;
    unsigned int charIndex = 0;

    constexpr static int CHARACTER_WIDTH  = 6;
    constexpr static int CHARACTER_HEIGHT = 8;
    constexpr static int OLED_WIDTH       = 128;
    constexpr static int OLED_HEIGHT      = 64;
    constexpr static int LINE_WIDTH       = OLED_WIDTH / CHARACTER_WIDTH;
    constexpr static int NUM_LINES        = OLED_HEIGHT / CHARACTER_HEIGHT;
    constexpr void PUT_AT_INDEX() { this->display.setCursorXY(CHARACTER_WIDTH * this->charIndex, CHARACTER_HEIGHT * this->lineIndex); }
    constexpr int MAP_X(const int x) { return x * CHARACTER_WIDTH; }
    constexpr int MAP_Y(const int y) { return y * CHARACTER_HEIGHT; }

    struct line
    {
        unsigned int length = 0;
        char str[LINE_WIDTH + 1] = {0};

        void copy(const line& l)
        {
            this->length = l.length;
            for (unsigned int i = 0; i < this->length; i++)
                this->str[i] = l.str[i];
        }

        void clear()
        {
            this->str[0] = 0;
            this->length = 0;
        }
    };

    line lines[NUM_LINES] = {0};

public:
    OLEDTerminal(GyverOLED<_TYPE, _BUFF, _CONN, _CS, _DC, _RST>& oled) : display(oled) {}

    ~OLEDTerminal()
    {}

    void print_char(const char c, bool update = true)
    {
        PUT_AT_INDEX();

        if (c == '\n')
        {
            this->lineIndex++;
            this->charIndex=0;
            PUT_AT_INDEX();
        }
        else
        {
            this->lines[this->lineIndex].str[this->charIndex] = c;
            this->lines[this->lineIndex].length = this->charIndex;

            this->display.write(c);

            this->charIndex++;
            if (this->charIndex > LINE_WIDTH)
            {
                this->lineIndex++;
                this->charIndex=0;

                if (this->lineIndex >= NUM_LINES)
                {
                    this->display.clear();
                    this->lineIndex = 0;
                    this->charIndex = 0;
                }
            }
        }

        if (update)
            this->display.update();
    }

    void print(const char* str, bool update = true)
    {
        while (*str != 0)
        {
            PUT_AT_INDEX();

            if (*str == '\n')
            {
                this->lineIndex++;
                this->charIndex=0;
                PUT_AT_INDEX();
            }
            else
            {
                this->lines[this->lineIndex].str[this->charIndex] = *str;
                this->lines[this->lineIndex].length = this->charIndex;

                this->display.write(*str);

                this->charIndex++;
                if (this->charIndex > LINE_WIDTH)
                {
                    this->lineIndex++;
                    this->charIndex=0;

                    if (this->lineIndex >= NUM_LINES)
                    {
                        this->display.clear();
                        this->lineIndex = 0;
                        this->charIndex = 0;
                    }
                }

            }

            str++;
        }

        if (update)
            this->display.update();
    }

    void println(const char* str, bool update = true)
    {
        this->print(str, update);

        this->lineIndex++;
        this->charIndex=0;
    }

    void set_cursor(int x, int y)
    {
        this->charIndex = x;
        this->lineIndex = y;
    }
};

struct StandardError : public EmbediFileStream
{    
    StandardError()
    {
        this->descriptor = {SYSERR_NAME, "", READ | WRITE};
    }

    bool write(const TypeMacros::u8 _byte) override;
    TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size) override;
};

extern StandardError errorStream;
extern GyverOLED<SSH1106_128x64> error_display;
extern OLEDTerminal<SSH1106_128x64> error_terminal;

// Implementation of SD Card reading
struct SDCardFileStream : public EmbediFileStream
{
    static SdFat card;
    SdFile file;

    SDCardFileStream(const char* path, const TypeMacros::u8 mode);
    ~SDCardFileStream();
    bool open() override;
    void close() override;
    bool write(const TypeMacros::u8 _byte) override;
    TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size) override;
    void flush() override;
    TypeMacros::u8 read() override;
    TypeMacros::u32 read(TypeMacros::u8* buffer, TypeMacros::u32 size) override;
    void seek(const TypeMacros::u32 pos) override;
    TypeMacros::u32 tell() override;
    TypeMacros::u32 available() override;
};

namespace SystemIO {
    bool open_file(EmbediFileStream* fileStream);
    void close_file(EmbediFileStream* fileStream);
    void close_file(const char* streamName);

    TypeMacros::u8 fgetc(const char* streamName);
    TypeMacros::u8 fgetc(EmbediFileStream* stream);

    TypeMacros::u32 favailable(const char* streamName);
    TypeMacros::u32 favailable(EmbediFileStream* stream);

    void fputc(const char* streamName, const TypeMacros::u8 c);
    void fputc(EmbediFileStream* stream, const TypeMacros::u8 c);

    void fflush(const char* streamName);
    void fflush(EmbediFileStream* stream);

    TypeMacros::u32 fputs(const char* streamName, const char* str);
    TypeMacros::u32 fputs(EmbediFileStream* stream, const char* str);

    TypeMacros::u32 fprintf(const char* streamName, const char* format, ...);
    TypeMacros::u32 fprintf(EmbediFileStream* stream, const char* format, ...);

    TypeMacros::u32 fgets(const char* streamName, char* buffer, TypeMacros::u32 capSize);
    TypeMacros::u32 fgets(EmbediFileStream* stream, char* buffer, TypeMacros::u32 capSize);

    TypeMacros::u8 getchar();

    TypeMacros::u32 available();

    TypeMacros::u32 gets(char* buffer, TypeMacros::u32 capSize);
    TypeMacros::u32 printf(const char* format, ...);

    void printchar(const TypeMacros::u8 c);
    void perror(const char* str);

    void flush();
};

#endif