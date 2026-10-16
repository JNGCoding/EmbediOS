#ifndef __EMBEDI_DISPLAY_DRIVER_HPP__
#define __EMBEDI_DISPLAY_DRIVER_HPP__

#include "../TypeMacros.hpp"
#include "../Defines.hpp"
#include "../DataStructures/DataStructures.hpp"
#include "../FileSystem/IOStreams.hpp"
#include "../Allocator/Allocator.hpp"
#include "Font.hpp"

#include <TFT_eSPI.h>
#include <stdlib.h>

#ifdef COMPILE_WITH_DISPLAY_INITIALIZATION

using namespace TypeMacros;

struct EmbediFont
{
    u16 char_width;
    u16 char_height;
    const u8* chars;
};

static EmbediFont DEFAULT_FONT =
{
    .char_width  = 8,
    .char_height = 12,
    .chars = DEFAULT_FONT_CHARS
};

class DrawableFrameBuffer
{
public:
    enum class Rotation
    {
        R0,
        R90,
        R180,
        R270,
    };

private:
    DisplayColor* buffer = nullptr;

    const u16 width;
    const u16 height;

    Rotation rotation = Rotation::R0;
    bool inverse = false;

    EmbediAllocator* allocator = nullptr;

    EmbediFont& font = DEFAULT_FONT;
    DisplayColor fontColor{};

    u16 cursorX = 0, cursorY = 0;

    constexpr u32 coords(const u16 x, const u16 y)
    { return this->width * y + x; }

    constexpr u32 line_index(const u16 y)
    { return this->height * y + 0; }

    const bool out_of_bounds(const u16 x, const u16 y) const
    { return x > this->vwidth() ||  y > this->vheight(); }

public:
    DrawableFrameBuffer(const u16 w, const u16 h, EmbediAllocator* alloc = &basicAllocator);
    ~DrawableFrameBuffer();

    // Set the rotation of the frame buffer
    void set_rotation(const Rotation r);

    // Set the inverted color flag of the frame buffer
    void set_inverse(const bool ni);
    
    // Set the text cursor of the frame buffer
    void set_cursor(const u16 x, const u16 y);

    // Set the font color of the frame buffer
    void set_font_color(const DisplayColor color);

    // Set the font of the frame buffer
    void set_font(EmbediFont& f);

    // Set the pixel value in the frame buffer at coordinates `(x, y)` with color `c`
    void draw_pixel(const u16 x, const u16 y, const DisplayColor c);

    // Draw a rect at `(x, y)` with size `(w, h)` with color `c` 
    void draw_rect(const u16 x, const u16 y, const u16 w, const u16 h, const DisplayColor c, const bool fill = false);

    // Draw a square at `(x, y)` with size `(side, side)` with color `c` 
    void draw_square(const u16 x, const u16 y, const u16 side, const DisplayColor c, const bool fill = false);

    // Draw a circle at `(x, y)` with radius `r` with color `c` 
    void draw_circle(const u16 x, const u16 y, const u16 r, const DisplayColor c, const bool fill = false);

    // Draw a line starting from `(x, y)` ending at `(x1, y1)` with color `c` 
    void draw_line(const u16 x, const u16 y, const u16 x1, const u16 y1, const DisplayColor c);

    // Draw a vertical line starting from `(x, y)` ending at `(x, y + h)` with color `c` 
    void draw_vline(const u16 x, const u16 y, const u16 h, const DisplayColor c);

    // Draw a horizontal line starting from `(x, y)` ending at `(x + w, y)` with color `c` 
    void draw_hline(const u16 x, const u16 y, const u16 w, const DisplayColor c);

    // Draw an ascii character on the screen
    void draw_char(const char c);

    // Draw text onto the screen
    void draw_text(const char* cstr);

    // Clear the screen with color `c`
    void fill_screen(const DisplayColor c);

    // Get the color value at `(x, y)` coordinates
    const DisplayColor get_color(u16 x, u16 y) const;

    // Get the pixel value at `(x, y)` coordinates
    const DisplayColor get_value(u16 x, u16 y) const;

    // Returns the effective width of the frame buffer
    const u16 vwidth() const;

    // Returns the effective height of the frame buffer
    const u16 vheight() const;
};

class EmbediDisplay
{
protected:
    DrawableFrameBuffer& fb;
    u16 currentLine = 0;

    const u16 displayWidth;
    const u16 displayHeight;

    // Override this function in order to implement the `update_next_line()` function of Display
    virtual void post_line() = 0;

    // Override this function in order to implement the `update()` function of Display
    virtual void post_fb() = 0;

public:
    EmbediDisplay(const u16 dw, const u16 dh, DrawableFrameBuffer& _fb);
    virtual ~EmbediDisplay() = default;

    // Meant to be used in the event loop, non-blockingly draws the next line of the display
    void update_next_line();

    // Updates the screen with the entire frame buffer all at once
    void update();
};

#endif

#endif