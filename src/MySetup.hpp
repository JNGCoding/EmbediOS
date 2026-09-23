// #ifndef __MY_SETUP_HPP__
// #define __MY_SETUP_HPP__

// #include <Arduino.h>
// #include <string.h>
// #include <Adafruit_SH110X.h>

// #include "Defines.hpp"
// #include "TypeMacros.hpp"
// #include "IOStreams.hpp"
// #include "WirelessConnectivity.hpp"
// #include "WiredConnectivity.hpp"

// #include "assert.h"

// class OLEDTerminal
// {
// private:
//     Adafruit_SH1106G& display;
//     TypeMacros::u8 lineIndex = 0;
//     TypeMacros::u8 charIndex = 0;

//     constexpr static TypeMacros::u8 CHARACTER_WIDTH  = 6;
//     constexpr static TypeMacros::u8 CHARACTER_HEIGHT = 8;
//     constexpr static TypeMacros::u8 OLED_WIDTH       = 64;     // These are settings set for the oled being in PORTRAIT form
//     constexpr static TypeMacros::u8 OLED_HEIGHT      = 128;
//     constexpr static TypeMacros::u8 LINE_WIDTH       = (OLED_WIDTH / CHARACTER_WIDTH) - 1;
//     constexpr static TypeMacros::u8 NUM_LINES        = OLED_HEIGHT / CHARACTER_HEIGHT;
//     constexpr TypeMacros::u8 MAP_X(const TypeMacros::u8 x) { return x * CHARACTER_WIDTH; }
//     constexpr TypeMacros::u8 MAP_Y(const TypeMacros::u8 y) { return y * CHARACTER_HEIGHT; }

//     void PUT_AT_INDEX() { this->display.setCursor(CHARACTER_WIDTH * this->charIndex, CHARACTER_HEIGHT * this->lineIndex); }
   
//     struct line
//     {
//         unsigned int length = 0;
//         char str[LINE_WIDTH + 1] = {0};

//         void copy(const line& l)
//         {
//             this->length = l.length;
//             for (unsigned int i = 0; i < this->length; i++)
//                 this->str[i] = l.str[i];
//         }

//         void clear()
//         {
//             this->str[0] = 0;
//             this->length = 0;
//         }
//     };

//     line lines[NUM_LINES];

// public:
//     OLEDTerminal(Adafruit_SH1106G& oled) : display(oled)
//     {
//     }

//     void printc(const char c, bool update = true)
//     {
//         PUT_AT_INDEX();

//         if (c == '\n')
//         {
//             this->lineIndex++;
//             this->charIndex=0;
//             PUT_AT_INDEX();
//         }
//         else
//         {
//             if (this->lineIndex >= NUM_LINES)
//             {
//                 this->display.clearDisplay();
//                 this->lineIndex = 0;
//                 this->charIndex = 0;
//                 PUT_AT_INDEX();
//             }

//             this->lines[this->lineIndex].str[this->charIndex] = c;
//             this->lines[this->lineIndex].length = this->charIndex;

//             this->display.write(c);

//             this->charIndex++;
//             if (this->charIndex > LINE_WIDTH)
//             {
//                 this->lineIndex++;
//                 this->charIndex=0;
//             }
//         }

//         if (update)
//             this->display.display();
//     }

//     void print(const char* str, bool update = true)
//     {
//         while (*str != 0)
//         {
//             this->printc(*str);
//             str++;
//         }

//         if (update)
//             this->display.display();
//     }

//     void println(const char* str, bool update = true)
//     {
//         this->print(str, update);
//         this->lineIndex++;
//         this->charIndex=0;
//     }

//     void set_cursor(TypeMacros::u8 x, TypeMacros::u8 y)
//     {
//         this->charIndex = embedi_constrain(x, 0, LINE_WIDTH);
//         this->lineIndex = embedi_constrain(y, 0, NUM_LINES);
//     }

//     TypeMacros::u16 read_text_buffer(char* buffer, TypeMacros::u16 size)
//     {
//         TypeMacros::u16 index = size - 1, charactersPrinted = 0;
//         buffer[index--] = 0;

//         for (TypeMacros::u8 y = 0; y <= this->lineIndex; y++)
//         {
//             line& working_line = this->lines[this->lineIndex - y];
//             unsigned int line_length = working_line.length;

//             for (int x = 0; x < line_length; x++)
//             {
//                 char c = working_line.str[line_length - x];
//                 if (index == 0)
//                     return charactersPrinted;

//                 buffer[index--] = c;
//                 charactersPrinted++;
//             }
//         }

//         return charactersPrinted;
//     }

//     void bus_flush()
//     {
//         this->display.flush();
//         this->display.display();
//     }

//     TypeMacros::u16 get_text_size()
//     {
//         return this->lineIndex * LINE_WIDTH + this->charIndex;
//     }
// };

// class ArduinoDisplayTerminal
// {
// private:
//     static constexpr TypeMacros::u8 RESET                      = 0x00;
//     static constexpr TypeMacros::u8 SET_CURSOR                 = 0x01;
//     static constexpr TypeMacros::u8 SET_COLOR                  = 0x02;
//     static constexpr TypeMacros::u8 SET_CURSOR_ENABLE          = 0x03;
//     static constexpr TypeMacros::u8 SET_CURSOR_BLINK           = 0x04;
//     static constexpr TypeMacros::u8 WRITE_STRING               = 0x05;
//     static constexpr TypeMacros::u8 WRITE_STRING_WITH_NEW_LINE = 0x06;
//     static constexpr TypeMacros::u8 WRITE_BACKSPACE            = 0x07;
//     static constexpr TypeMacros::u8 SCROLL_UP                  = 0x08;
//     static constexpr TypeMacros::u8 SCROLL_DOWN                = 0x09;
//     static constexpr TypeMacros::u8 CLEAR                      = 0x0A;
//     static constexpr TypeMacros::u8 GET_CURSOR                 = 0x0B;
//     static constexpr TypeMacros::u8 WRITE_CHARACTER            = 0x0C;
//     static constexpr TypeMacros::u8 SET_BACKGROUND_COLOR       = 0x0D;
//     static constexpr TypeMacros::u8 SET_HIGHLIGHT_COLOR        = 0x0E;
//     static constexpr TypeMacros::u8 GET_INFORMATION_COMMAND    = 0XF0;

//     HardwareSerial& databus;

//     TypeMacros::u16 charIndex = 0;
//     TypeMacros::u16 lineIndex = 0;

//     TypeMacros::u16 displayWidth = 240;
//     TypeMacros::u16 displayHeight = 320;

//     static constexpr TypeMacros::u16 CHARACTER_WIDTH = 6;
//     static constexpr TypeMacros::u16 CHARACTER_HEIGHT = 8;

//     const TypeMacros::u16 totalLines;
//     const TypeMacros::u16 lineWidth;

//     void PUT_AT_INDEX()
//     {
//         TypeMacros::u16 pixel_x = this->charIndex;
//         TypeMacros::u16 pixel_y = this->lineIndex;

//         TypeMacros::u8 payload[] = {
//             static_cast<TypeMacros::u8>((pixel_x >> 8) & 0xFF),
//             static_cast<TypeMacros::u8>(pixel_x & 0xFF),
//             static_cast<TypeMacros::u8>((pixel_y >> 8) & 0xFF),
//             static_cast<TypeMacros::u8>(pixel_y & 0xFF),
//         };
        
//         this->databus.write(SET_CURSOR);
//         this->databus.write(payload, 4 * sizeof(TypeMacros::u8));

//         delay(1);
//     }

//     void write(const char c)
//     {
//         this->databus.write(WRITE_CHARACTER);
//         this->databus.write(c);
//     }

//     void write(const char* str)
//     {
//         this->databus.write(WRITE_STRING);
//         this->databus.print(str);
//         this->databus.write('\r');
//     }

// public:
//     ArduinoDisplayTerminal(HardwareSerial& bus, const TypeMacros::u16 dw, const TypeMacros::u16 dh)
//         : databus(bus), lineWidth((dw / CHARACTER_WIDTH) - 1), totalLines(dh / CHARACTER_HEIGHT)
//     {
//         this->displayWidth = dw;
//         this->displayHeight = dh;
//     }

//     void printc(const char c, bool flush = true)
//     {
//         this->PUT_AT_INDEX();

//         if (c == '\n')
//         {
//             this->lineIndex++;
//             this->charIndex=0;
//             this->PUT_AT_INDEX();
//         }
//         else if (c == '\t')
//         {
//             this->printc(' ');
//             this->printc(' ');
//             this->printc(' ');
//             this->printc(' ');
//         }
//         else
//         {
//             if (this->lineIndex >= this->totalLines)
//             {
//                 this->clear();
//                 this->PUT_AT_INDEX();
//             }

//             this->write(c);

//             this->charIndex++;
//             if (this->charIndex > lineWidth)
//             {
//                 this->lineIndex++;
//                 this->charIndex=0;
//             }
//         }

//         if (flush)
//             this->bus_flush();
//     }

//     void print(const char* str, bool flush = true)
//     {
//         while (*str != 0)
//         {
//             this->printc(*str, false);
//             str++;
//         }

//         if (flush)
//             this->bus_flush();
//     }

//     void println(const char* str, bool flush = true)
//     {
//         this->print(str, flush);
//         this->lineIndex++;
//         this->charIndex=0;
//     }

//     void set_cursor(TypeMacros::u16 x, TypeMacros::u16 y)
//     {
//         this->charIndex = embedi_constrain(x, 0, this->lineWidth);
//         this->lineIndex = embedi_constrain(y, 0, this->totalLines);
//     }

//     TypeMacros::u16 get_cursor_x()
//     {
//         return this->charIndex;
//     }

//     TypeMacros::u16 get_cursor_y()
//     {
//         return this->lineIndex;
//     }

//     void set_text_color(TypeMacros::u16 color)
//     {
//         this->databus.write(SET_COLOR);
//         TypeMacros::u8 payload[] = {
//             static_cast<TypeMacros::u8>((color >> 8) & 0xFF),
//             static_cast<TypeMacros::u8>(color & 0xFF),
//         };

//         this->databus.write(payload, 2 * sizeof(TypeMacros::u8));
//         this->databus.flush();
//     }

//     void set_background_color(TypeMacros::u16 color)
//     {
//         this->databus.write(SET_BACKGROUND_COLOR);
//         TypeMacros::u8 payload[] = {
//             static_cast<TypeMacros::u8>((color >> 8) & 0xFF),
//             static_cast<TypeMacros::u8>(color & 0xFF),
//         };

//         this->databus.write(payload, 2 * sizeof(TypeMacros::u8));
//         this->databus.flush();
//     }

//     void set_highlight_color(TypeMacros::u16 color)
//     {
//         this->databus.write(SET_HIGHLIGHT_COLOR);
//         TypeMacros::u8 payload[] = {
//             static_cast<TypeMacros::u8>((color >> 8) & 0xFF),
//             static_cast<TypeMacros::u8>(color & 0xFF),
//         };

//         this->databus.write(payload, 2 * sizeof(TypeMacros::u8));
//         this->databus.flush();
//     }

//     void bus_flush()
//     {
//         this->databus.flush();
//     }

//     void reset_driver()
//     {
//         this->databus.write(RESET);
//         this->charIndex = 0;
//         this->lineIndex = 0;
//     }

//     void toggle_cursor()
//     {
//         this->databus.write(SET_CURSOR_ENABLE);
//         this->databus.write(1);
//         this->databus.flush();
//     }

//     void toggle_cursor_blink()
//     {
//         this->databus.write(SET_CURSOR_BLINK);
//         this->databus.write(1);
//         this->databus.flush();
//     }

//     void clear()
//     {
//         this->charIndex = 0;
//         this->lineIndex = 0;
//         this->databus.write(CLEAR);
//         this->databus.flush();
        
//         // This delay is important as Arduino takes some time
//         // filling the screen with background color
//         delay(200);
//     }
// };

// struct OLEDTerminalSYSError : public EmbediFileStream
// {
//     OLEDTerminal& display;

//     OLEDTerminalSYSError(OLEDTerminal& oled) : display(oled) {
//         this->descriptor = {SYSERR_NAME, "", READ | WRITE};
//     }

//     bool write(const TypeMacros::u8 _byte) override
//     {
//         this->display.printc(static_cast<char>(_byte), true);
//         return true;
//     }

//     TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size) override
//     {
//         this->display.print(reinterpret_cast<const char*>(_data), true);
//         return size;
//     }

//     void flush() override
//     {
//         this->display.bus_flush();
//     }

//     TypeMacros::u8 read() override
//     {
//         char buffer[2] = {0};
//         this->display.read_text_buffer(buffer, 2);
//         return buffer[0];
//     }

//     TypeMacros::u32 read(TypeMacros::u8* buffer, TypeMacros::u32 size) override
//     {
//         return this->display.read_text_buffer(reinterpret_cast<char*>(buffer), size);
//     }

//     TypeMacros::u32 available() override
//     {
//         return this->display.get_text_size();
//     }
// };

// struct ArduinoDisplayTerminalSYSOutput : public EmbediFileStream
// {
//     ArduinoDisplayTerminal& display;

//     ArduinoDisplayTerminalSYSOutput(ArduinoDisplayTerminal& lcd) : display(lcd)
//     {
//         this->descriptor = {SYSOUT_NAME, "", WRITE};
//     }

//     bool write(const TypeMacros::u8 _byte) override
//     {
//         this->display.printc(static_cast<char>(_byte), true);
//         return true;
//     }

//     TypeMacros::u32 write(const TypeMacros::u8* _data, TypeMacros::u32 size) override
//     {
//         this->display.print(reinterpret_cast<const char*>(_data), true);
//         return size;
//     }

//     void flush() override
//     {
//         this->display.bus_flush();
//     }
// };

// struct ArduinoDisplayTerminal_BluetoothSYSOutput : public EmbediFileStream
// {
//     ArduinoDisplayTerminalSYSOutput arduino_terminal;
//     EmbediBluetoothSerialOutput bluetooth_terminal;

//     ArduinoDisplayTerminal_BluetoothSYSOutput(ArduinoDisplayTerminal& lcd) : arduino_terminal(lcd)
//     {
//         this->descriptor = {SYSOUT_NAME, "", WRITE};
//     }

//     bool write(const TypeMacros::u8 _byte) override
//     {
//         this->arduino_terminal.write(_byte);
//         this->bluetooth_terminal.write(_byte);
//         return true;
//     }

//     void flush() override
//     {
//         this->arduino_terminal.flush();
//         this->bluetooth_terminal.flush();
//     }
// };

// #endif