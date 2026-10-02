#include "Display.hpp"

#ifdef COMPILE_WITH_DISPLAY_INITIALIZATION

EmbediDisplay::EmbediDisplay()
{}

void EmbediDisplay::set_rotation(const Rotation r)
{
    this->rotated = r;
}

void EmbediDisplay::set_inverse(const bool ni)
{
    this->inverse = ni;
}

void EmbediDisplay::set_cursor(const u16 x, const u16 y)
{
    this->cursorX = x;
    this->cursorY = y;
}

void EmbediDisplay::set_font_color(const DisplayColor color)
{
    this->fontColor = color;
}

const DisplayColor& EmbediDisplay::get_pixel(const u16 x, const u16 y)
{
    return *this->frameBuffer.get(this->coords(x, y));
}

void EmbediDisplay::pixel(const u16 x, const u16 y, const DisplayColor color)
{
    if (this->out_of_bounds(x, y))
        return;
    
    switch (this->rotated)
    {
        case R0:
            this->frameBuffer.place(this->coords(x, y), (this->inverse ? ~color : color));
            break;
            
        case R90:
            this->frameBuffer.place(this->coords(this->width() - 1 - y, x), (this->inverse ? ~color : color));
            break;

        case R180:
            this->frameBuffer.place(this->coords(this->width() - 1 - x, this->height() - 1 - y), (this->inverse ? ~color : color));
            break;

        case R270:
            this->frameBuffer.place(this->coords(y, this->height() - 1 - x), (this->inverse ? ~color : color));
            break;

        default:
            this->frameBuffer.place(this->coords(x, y), (this->inverse ? ~color : color));
    }
}

void EmbediDisplay::printc(const char character)
{
    if (this->cursorX > this->width())
        return;
    
    if (this->cursorY > this->height())
        return;

    const u16 bytesPerRow = ceil(this->font.char_width / 8);
    const u8* charPtr = this->font.chars + (static_cast<int>(character) * this->font.char_height * bytesPerRow);

    u16 currentChar = 0;

    u16 dx = this->cursorX;
    u16 dy = this->cursorY;

    for (u16 row = 0; row < this->font.char_height; row++)
    {
        u16 bitsProcessed = 0;

        for (u16 byteInd = 0; byteInd < bytesPerRow; byteInd++)
        {
            u8 byte = charPtr[byteInd + (row * this->font.char_height)];

            for (u16 i = 0; i < embedi_min(this->font.char_width, 8); i++)
            {
                if (bitsProcessed >= this->font.char_width)
                    break;

                if ((byte & (1 << i)) != 0)
                    this->pixel(dx, dy, this->fontColor);
                
                dx++;
                bitsProcessed++;
            }
        }

        dy++;
    }

    this->cursorX += this->font.char_width + 1;
    this->cursorY += this->font.char_height + 1;
}

void EmbediDisplay::print(const char* text)
{
    while (*text != 0)
    {
        this->printc(*text);
        text++;
    }
}

void EmbediDisplay::println(const char* text)
{
    this->print(text);
    this->printc('\n');
}

void EmbediDisplay::rect(const u16 x, const u16 y, const u16 w, const u16 h, const bool fill,const DisplayColor color)
{
    if (this->out_of_bounds(x, y))
        return;

    if (w == 0 || h == 0)
        return;
    
    u16 fx = (w + x > this->width() ? this->width() : w + x);
    u16 fy = (h + y > this->height() ? this->width() : h + y);

    if (fill)
    {
        for (u16 dx = x; dx < fx; dx++)
        {
            for (u16 dy = y; dy < fy; dy++)
            {
                this->pixel(dx, dy, color);
            }
        }
    }
    else
    {
        this->hline(x, y, fx - 1, false, color);
        this->hline(x, fy, fx - 1, false, color);
        this->vline(x, y, fy - 1, false, color);
        this->vline(fx, y, fy - 1, false, color);
    }
}

void EmbediDisplay::square(const u16 x, const u16 y, const u16 s, const bool fill, const DisplayColor color)
{
    this->rect(x, y, s, s, fill, color);
}

void EmbediDisplay::circle(const u16 x, const u16 y, const u16 r, const bool fill,const DisplayColor color)
{
    if (this->out_of_bounds(x, y))
        return;

    if (r == 0)
        return;

    u16 dx = 0;
    u16 dy = r;
    i16 d = 3 - 2 * r;

    auto plotCirclePoints = [&](u16 cx, u16 cy, u16 px, u16 py) {
        if (fill)
        {
            while (dx <= dy)
            {
                this->hline(x - dx, y + dy, 2 * dx + 1, false, color);
                this->hline(x - dx, y - dy, 2 * dx + 1, false, color);
                this->hline(x - dx, y + dy, 2 * dy + 1, false, color);
                this->hline(x - dx, y - dy, 2 * dy + 1, false, color);
            }

            if (d > 0)
            {
                dy--;
                d = d + 4 * static_cast<i16>(dx - dy) + 10;
            }
            else
            {
                d = d + 4 * static_cast<i16>(dx) + 6;
            }

            dx++;
        }
        else
        {
            this->pixel(cx + px, cy + py, color);
            this->pixel(cx - px, cy + py, color);
            this->pixel(cx + px, cy - py, color);
            this->pixel(cx - px, cy - py, color);
            this->pixel(cx + py, cy + px, color);
            this->pixel(cx + py, cy - px, color);
            this->pixel(cx - py, cy + px, color);
            this->pixel(cx - py, cy - px, color);
        }
    };

    plotCirclePoints(x, y, dx, dy);

    while (dx <= dy)
    {
        dx++;

        if (d > 0)
        {
            dy--;
            d = d + 4 * static_cast<i16>(dx - dy) + 10;
        }
        else
        {
            d = d + 4 * static_cast<i16>(dy) + 6;
        }

        plotCirclePoints(x, y, dx ,dy);
    }
}

void EmbediDisplay::line(const u16 x, const u16 y, const u16 x1, const u16 y1,const DisplayColor color)
{
    const i16 deltax = embedi_abs(static_cast<i16>(x1) - static_cast<i16>(x));
    const i16 deltay = -embedi_abs(static_cast<i16>(y1) - static_cast<i16>(y));

    i16 sx = 1, sy = 1;

    if (x > x1)
        sx = -1;
    
    if (y > y1)
        sy = -1;

    i16 err = deltax + deltay;
    u16 dx = x, dy = y, fx = x1, fy = y1;

    while (true)
    {
        this->pixel(x, y, color);
        if (dx == fx && dy == fy)
            break;

        i16 e2 = 2 * err;

        if (e2 >= deltay)
        {
            err += deltay;
            fx += sx;
        }
        else
        {
            err += deltax;
            fy == sy;
        }
    }
}

void EmbediDisplay::hline(const u16 x, const u16 y, const u16 w, const bool reversed,const DisplayColor color)
{
    if (w == 0)
        return;
        
    if (reversed)
    {
        const u16 fx = (w + x > this->width() ? this->width() : w + x);

        for (u16 dx = x; dx < fx; dx++)
            this->pixel(dx, y, color);
    }
    else if (reversed)
    {
        const u16 fx = (x - w <= 0 ? 0 : x - w);

        for (i16 dx = x; dx < fx; dx--)
        {
            this->pixel(dx, y, color);
            if (dx == 0) break;
        }
    }
}

void EmbediDisplay::vline(const u16 x, const u16 y, const u16 h, const bool reversed,const DisplayColor color)
{
    if (h == 0)
        return;

    if (reversed)
    {
        const u16 fy = (h + y > this->height() ? this->height() : h + y);

        for (u16 dy = y; dy < fy; dy++)
            this->pixel(x, dy, color);
    }
    else
    {
        const u16 fy = (y - h < 0 ? 0 : y - h);

        for (u16 dy = y; dy < fy; dy--)
        {
            this->pixel(x, dy, color);
            if (dy == 0) break;
        }
    }
}


void EmbediDisplay::draw_next_line()
{
    this->display.pushImage(0, this->currentLine, DISPLAY_WIDTH, 1, this->frameBuffer.get_buf());
    this->currentLine = (this->currentLine + 1) % DISPLAY_HEIGHT;
}

void EmbediDisplay::update()
{
    this->display.pushImage(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->frameBuffer.get_buf());
    this->currentLine = 0;
}

void EmbediDisplay::clear(const DisplayColor color)
{
    this->frameBuffer.objset(color);
}

// This function returns the virtual width of the screen
const u16 EmbediDisplay::width()
{
    switch (this->rotated)
    {
        case R0:
            return DISPLAY_WIDTH;
        
        case R90:
            return DISPLAY_HEIGHT;
        
        case R180:
            return DISPLAY_WIDTH;

        case R270:
            return DISPLAY_HEIGHT;

        default:
            return DISPLAY_WIDTH;
    }
}

// This function returns the virtual height of the screen
const u16 EmbediDisplay::height()
{
    switch (this->rotated)
    {
        case R0:
            return DISPLAY_HEIGHT;
        
        case R90:
            return DISPLAY_WIDTH;
        
        case R180:
            return DISPLAY_HEIGHT;

        case R270:
            return DISPLAY_WIDTH;

        default:
            return DISPLAY_HEIGHT;
    }
}


#endif