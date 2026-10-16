#ifndef __SH1106_OLED_DRIVER_HPP__
#define __SH1106_OLED_DRIVER_HPP__

#include <math.h>
#include <FileSystem/IOStreams.hpp>
#include <Connectivity/WiredConnectivity.hpp>
#include <Drivers/Display.hpp>

class EmbediSH1106 : public EmbediDisplay
{
private:
    EmbediWireStream wire;

    static constexpr u8 SET_CONTRAST            = 0x81;
    static constexpr u8 DISPLAY_ALL_ON_RESUME   = 0xA4;
    static constexpr u8 DISPLAY_ALL_ON          = 0xA5;
    static constexpr u8 NORMAL_DISPLAY          = 0xA6;
    static constexpr u8 INVERT_DISPLAY          = 0xA7;
    static constexpr u8 DISPLAY_OFF             = 0xAE;
    static constexpr u8 DISPLAY_ON              = 0xAF;
    static constexpr u8 SET_DISPLAY_OFFSET      = 0xD3;
    static constexpr u8 SET_COMP_INS            = 0xDA;
    static constexpr u8 SET_VCOM_DETECT         = 0xD8;
    static constexpr u8 SET_DISPLAY_CLOCK_DIV   = 0xD5;
    static constexpr u8 SET_PRECHARGE           = 0xD9;
    static constexpr u8 SET_MULTIPLEX           = 0xA8;
    static constexpr u8 SET_COLUMN_ADDRLOW      = 0x00;
    static constexpr u8 SET_COLUMN_ADDRHIGH     = 0x10;
    static constexpr u8 SET_START_LINE          = 0x40;
    static constexpr u8 MEMORY_MODE             = 0x20;
    static constexpr u8 PAGEADDR                = 0x80;
    static constexpr u8 COMSCANINC              = 0xC0;
    static constexpr u8 COMSCANDEC              = 0xC8;
    static constexpr u8 SEGREMAP                = 0xA0;
    static constexpr u8 CHARGE_PUMP             = 0x8D;
    static constexpr u8 EXTERNAL_VCC            = 0x01;
    static constexpr u8 SWITCH_CAPVCC           = 0x02;
    static constexpr u8 MESSAGE_TYPE_COMMAND    = 0x80;
    static constexpr u8 MESSAGE_TYPE_DATA       = 0x40;
    static constexpr u8 READ_MODIFY_WRITE_START = 0xE0;
    static constexpr u8 READ_MODIFY_WRITE_END   = 0xEE;
    static constexpr u8 NUMBER_OF_SEGMENTS      = 0x80;
    static constexpr u8 NUMBER_OF_PAGES         = 0x08;
    static constexpr u8 ROWHEIGHT               = 0x08;
    static constexpr u8 MAX_SEGMENTS_PER_WRITE  = 0x10;

    u8 px = 0, py = 0;

    void write_command(u8 command)
    {
        this->wire.write(MESSAGE_TYPE_COMMAND);
        this->wire.write(command);
    }

    void write_data(u8 data)
    {
        this->wire.write(MESSAGE_TYPE_DATA);
        this->wire.write(data);
    }

    void set_write_position(u8 x, u8 y)
    {
        if (x == this->px && y == this->py)
            return;
        
        this->px = x;
        this->py = y >> 3;

        x += 2;

        this->write_command(PAGEADDR + (y >> 3));
        this->write_command(x & 0x0F);
        this->write_command(SET_COLUMN_ADDRHIGH + (x >> 4));

        this->wire.flush();
    }

protected:
    void post_line() override
    {
        // const u8 page = static_cast<u8>(ceil(this->currentLine / NUMBER_OF_PAGES));
        this->set_write_position(0, this->currentLine);
        
        for (u8 segment = 0; segment < (NUMBER_OF_SEGMENTS / MAX_SEGMENTS_PER_WRITE); segment++)
        {
            for (u8 part = 0; part < MAX_SEGMENTS_PER_WRITE; part++)
                break;
        }
    }

    void post_fb() override
    {
        for (u8 page = 0; page < NUMBER_OF_PAGES; page++)
        {
            this->set_write_position(0, page * ROWHEIGHT);
            for (u8 segment = 0; segment < (NUMBER_OF_SEGMENTS / MAX_SEGMENTS_PER_WRITE); segment++)
            {
                for (u8 part = 0; part < MAX_SEGMENTS_PER_WRITE; part++)
                    this->write_data(this->fb.get_value(segment + part, page * ROWHEIGHT));
            }
        }

        this->wire.flush();
    }

public:
    EmbediSH1106(DrawableFrameBuffer& _fb) :
        EmbediDisplay(128, 64, _fb),
        wire("SH1106", 0x3C, 64)
    {
        this->wire.set_post_policy(PostingPolicy::MANUAL_POST);
        this->wire.set_poll_policy(PollingPolicy::MANUAL_POLL);
    }

    ~EmbediSH1106()
    {
        if (this->wire.opened())
            SystemIO::close_file(&this->wire);
    }

    void init()
    {
        if (!EmbediWireManager::is_initialized())
            EmbediWireManager::init_i2c(EmbediWireConfig{Kbps(400), ChannelType::SENDER, 0x00});

        SystemIO::open_file(&this->wire);

        // Write initialization code
        this->write_command(DISPLAY_OFF);
        this->write_command(SET_DISPLAY_CLOCK_DIV);
        this->write_command(0x80);
        this->write_command(SET_MULTIPLEX);
        this->write_command(0x3F);
        this->write_command(SET_DISPLAY_OFFSET);
        this->write_command(0x00);
        this->write_command(SET_START_LINE | 0x00);
        this->write_command(CHARGE_PUMP);
        this->write_command(0x10);
        this->write_command(MEMORY_MODE);
        this->write_command(0x00);
        this->write_command(SEGREMAP | 0x1);
        this->write_command(COMSCANDEC);
        this->write_command(SET_COMP_INS);
        this->write_command(0x12);
        this->write_command(SET_CONTRAST);
        this->write_command(0x9F);
        this->write_command(SET_PRECHARGE);
        this->write_command(0x22);
        this->write_command(SET_VCOM_DETECT);
        this->write_command(0x40);
        this->write_command(DISPLAY_ALL_ON_RESUME);
        this->write_command(NORMAL_DISPLAY);
        this->write_command(DISPLAY_ON);
        this->write_command(SET_COLUMN_ADDRLOW | 0x0);
        this->write_command(SET_COLUMN_ADDRHIGH | 0x0);
        this->write_command(SET_START_LINE | 0x0);
        this->wire.flush();
    }
};

#endif
