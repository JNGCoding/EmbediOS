#ifndef __INTERNAL_FUNCTIONS_HPP__
#define __INTERNAL_FUNCTIONS_HPP__

#include "Defines.hpp"
#include "TypeMacros.hpp"

using namespace TypeMacros;

// We have to implement some shit now here
#ifdef COMPILE_WITH_DISPLAY_INITIALIZATION

// Display Cursor Position
struct DCPos
{
    u32 x, y;
};

extern void clear_display();
extern void set_cursor_position(u32 x, u32 y);
extern DCPos get_cursor_position();
extern void print_text(const char* text);
extern void println_text(const char* text);
extern void set_text_color(u16 color);
extern void set_highlight_color(u16 color);
extern void set_background_color(u16 color);

#endif

#endif