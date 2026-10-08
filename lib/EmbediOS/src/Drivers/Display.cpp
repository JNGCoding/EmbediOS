#include "Display.hpp"

#ifdef COMPILE_WITH_DISPLAY_INITIALIZATION

DrawableFrameBuffer::DrawableFrameBuffer(const u16 w, const u16 h, EmbediAllocator* alloc = &basicAllocator) : width(w), height(h)
{
    this->allocator = (alloc == nullptr ? &basicAllocator : alloc);
    if (alloc == nullptr)
        SystemIO::perror("[DrawableFrameBuffer::DrawableFrameBuffer(u16, u16)] ERROR : ALLOCATOR PROVIDED WAS NULL, DEFAULTING TO basicAllocator.");

    this->buffer = static_cast<DisplayColor*>(this->allocator->alloc(sizeof(DisplayColor) * this->width * this->height));
    if (this->buffer)
        SystemIO::perror("[DrawableFrameBuffer::DrawableFrameBuffer(u16, u16)] ERROR : FAILED TO ALLOCATE FRAMEBUFFER, ALL OPERATIONS OF FRAMEBUFFER ARE USELESS");
}

DrawableFrameBuffer::~DrawableFrameBuffer()
{
    if (this->buffer != nullptr)
        this->allocator->destroy(this->buffer);
}

void DrawableFrameBuffer::set_rotation(const Rotation r)
{
    this->rotation = r;
}

void DrawableFrameBuffer::set_inverse(const bool ni)
{
    this->inverse = ni;
}

void DrawableFrameBuffer::set_cursor(const u16 x, const u16 y)
{
    this->cursorX = x;
    this->cursorY = y;
}

void DrawableFrameBuffer::set_font_color(const DisplayColor color)
{
    this->fontColor = color;
}

void DrawableFrameBuffer::set_font(EmbediFont& f)
{
    this->font = f;
}

void DrawableFrameBuffer::draw_pixel(const u16 x, const u16 y, const DisplayColor c)
{
    if (this->out_of_bounds(x, y))
        return;
    
    switch (this->rotation)
    {
        case Rotation::R0:
            this->buffer[this->coords(x, y)] = (this->inverse ? ~c : c);
            break;
            
        case Rotation::R90:
            this->buffer[this->coords(this->vwidth() - 1 - y, x)] = (this->inverse ? ~c : c);
            break;

        case Rotation::R180:
            this->buffer[this->coords(this->vwidth() - 1 - x, this->vheight() - 1 - y)] = (this->inverse ? ~c : c);
            break;

        case Rotation::R270:
            this->buffer[this->coords(y, this->vheight() - 1 - x)] = (this->inverse ? ~c : c);
            break;

        default:
            this->buffer[this->coords(x, y)] = (this->inverse ? ~c : c);
            break;
    }
}

void DrawableFrameBuffer::draw_rect(const u16 x, const u16 y, const u16 w, const u16 h, const DisplayColor c, const bool fill = false)
{
    if (this->out_of_bounds(x, y))
        return;

    if (w == 0 || h == 0)
        return;
    
    u16 fx = (w + x > this->vwidth() ? this->vwidth() : w + x);
    u16 fy = (h + y > this->vheight() ? this->vheight() : h + y);

    if (fill)
    {
        for (u16 dx = x; dx < fx; dx++)
            for (u16 dy = y; dy < fy; dy++)
                this->draw_pixel(dx, dy, c);
    }
    else
    {
        this->draw_hline(x, y, fx - 1, c);
        this->draw_hline(x, fy, fx - 1, c);
        this->draw_vline(x, y, fy - 1, c);
        this->draw_vline(fx, y, fy - 1, c);
    }
}

void DrawableFrameBuffer::draw_square(const u16 x, const u16 y, const u16 side, const DisplayColor c, const bool fill = false)
{
    this->draw_rect(x, y, side, side, c, fill);
}

void DrawableFrameBuffer::draw_circle(const u16 x, const u16 y, const u16 r, const DisplayColor c, const bool fill = false)
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
                this->draw_hline(x - dx, y + dy, 2 * dx + 1, c);
                this->draw_hline(x - dx, y - dy, 2 * dx + 1, c);
                this->draw_hline(x - dx, y + dy, 2 * dy + 1, c);
                this->draw_hline(x - dx, y - dy, 2 * dy + 1, c);
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
            this->draw_pixel(cx + px, cy + py, c);
            this->draw_pixel(cx - px, cy + py, c);
            this->draw_pixel(cx + px, cy - py, c);
            this->draw_pixel(cx - px, cy - py, c);
            this->draw_pixel(cx + py, cy + px, c);
            this->draw_pixel(cx + py, cy - px, c);
            this->draw_pixel(cx - py, cy + px, c);
            this->draw_pixel(cx - py, cy - px, c);
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

void DrawableFrameBuffer::draw_line(const u16 x, const u16 y, const u16 x1, const u16 y1, const DisplayColor c)
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
        this->draw_pixel(x, y, c);
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

void DrawableFrameBuffer::draw_vline(const u16 x, const u16 y, const u16 h, const DisplayColor c)
{
    if (h == 0)
        return;

    const u16 fy = (y + h >= this->vheight() ? this->vheight() : y + h);

    for (u16 dy = y; dy < fy; dy++)
        this->draw_pixel(x, dy, c);
}

void DrawableFrameBuffer::draw_hline(const u16 x, const u16 y, const u16 w, const DisplayColor c)
{
    if (w == 0)
        return;

    const u16 fx = (x + w >= this->vwidth() ? this->vwidth() : x + w);

    for (i16 dx = x; dx < fx; dx++)
        this->draw_pixel(dx, y, c);
}

void DrawableFrameBuffer::draw_char(const char c)
{
    if (this->cursorX > this->vwidth())
        return;
    
    if (this->cursorY > this->vheight())
        return;

    const u16 bytesPerRow = ceil(this->font.char_width / 8);
    const u8* charPtr = this->font.chars + (static_cast<int>(c) * this->font.char_height * bytesPerRow);

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
                    this->draw_pixel(dx, dy, this->fontColor);
                
                dx++;
                bitsProcessed++;
            }
        }

        dy++;
    }

    this->cursorX += this->font.char_width + 1;
    this->cursorY += this->font.char_height + 1;
}

void DrawableFrameBuffer::draw_text(const char* cstr)
{
    while (*cstr != 0)
    {
        this->draw_char(*cstr);
        cstr++;
    }
}

void DrawableFrameBuffer::fill_screen(const DisplayColor c)
{
    memset(this->buffer, c, sizeof(c) * this->vwidth() * this->vheight());
}

const u16 DrawableFrameBuffer::vwidth() const
{
    switch (this->rotation)
    {
        case Rotation::R0:
            return this->width;
        
        case Rotation::R90:
            return this->height;
        
        case Rotation::R180:
            return this->width;

        case Rotation::R270:
            return this->height;

        default:
            return this->width;
    }
}

const u16 DrawableFrameBuffer::vheight() const
{
    switch (this->rotation)
    {
        case Rotation::R0:
            return this->height;
        
        case Rotation::R90:
            return this->width;
        
        case Rotation::R180:
            return this->height;

        case Rotation::R270:
            return this->width;

        default:
            return this->height;
    }
}

EmbediDisplay::EmbediDisplay(const u16 dw, const u16 dh, DrawableFrameBuffer& _fb) : displayWidth(dw), displayHeight(dh), fb(_fb)
{}

void EmbediDisplay::update_next_line()
{
    this->post_line();
    this->currentLine = (this->currentLine + 1) % this->displayHeight;
}

void EmbediDisplay::update()
{
    this->post_fb();
    this->currentLine = 0;
}

#endif