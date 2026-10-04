#ifndef WHAT_THE_FUCK
#define WHAT_THE_FUCK

#include <Arduino.h>
#include <driver/uart.h>
#include <FileSystem/IOStreams.hpp>

// This is only Out, not In
class HardwareUART : public EmbediFileStream
{
public:
    HardwareUART(const TypeMacros::u32 bufferSize = 1024) : EmbediFileStream(bufferSize)
    {}

    bool open() override
    {
        uart_config_t config = {
            .baud_rate = 115200,
            .data_bits = uart_word_length_t::UART_DATA_8_BITS,
            .parity    = uart_parity_t::UART_PARITY_ODD,
            .stop_bits = uart_stop_bits_t::UART_STOP_BITS_1,
            .flow_ctrl = uart_hw_flowcontrol_t::UART_HW_FLOWCTRL_DISABLE,
            .source_clk = uart_sclk_t::UART_SCLK_APB
        };

        uart_param_config(UART_NUM_0, &config);
        uart_set_pin(UART_NUM_0, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    }

    void post(const TypeMacros::u32 n = IO_POST_SPEED) override
    {
        const TypeMacros::u32 bytesToPost = embedi_min(n, this->writeQueue.size());
        for (int i = 0; i < bytesToPost; i++)
        {
            TypeMacros::u8* data = this->writeQueue.dequeue();
            if (data != nullptr) {
                uart_tx_chars(UART_NUM_0, reinterpret_cast<char*>(data), 1);
            }
        }
    }

    void poll() override
    {}
};

#endif