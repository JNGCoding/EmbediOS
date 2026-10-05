#include "WirelessConnectivity.hpp"

#ifdef COMPILE_WITH_BLUETOOTH_INITIALIZATION

BluetoothSerial __Internal_BTSerial;
EmbediBluetoothConfig __Internal_config = {0};
bool __Internal_BluetoothActivated = false;

EmbediBluetoothStream::EmbediBluetoothStream(const TypeMacros::u32 bufferSize) : EmbediFileStream(bufferSize)
{
    this->descriptor = {
        "EmbediBluetoothDuplexStream",
        ".sys/bluetooth/io/serial_stream",
        WRITE | READ
    };
}

bool EmbediBluetoothStream::open()
{
    this->isOpened = this->allocate_buffers();
    return this->isOpened;
}

void EmbediBluetoothStream::post(const TypeMacros::u32 n)
{
    if (this->tx_ready() && __Internal_BluetoothActivated)
    {
        const TypeMacros::u32 toPost = embedi_min(this->writeQueue.size(), n);
        for (TypeMacros::u32 i = 0; i < toPost; i++)
        {
            TypeMacros::u8* data = this->writeQueue.dequeue();
            if (data == nullptr)
            {
                this->error = ErrorCode::NULLPTR_ENCOUNTERED;
            }
            else
            {
                this->error = ErrorCode::NO_ERROR;
                __Internal_BTSerial.write(*data);
            }
        }
    }
    else this->error = ErrorCode::WRITE_BUFFER_NULL;

}

void EmbediBluetoothStream::poll()
{
    if (this->rx_ready() && __Internal_BluetoothActivated)
    {
        while (__Internal_BTSerial.available() > 0)
            this->readRingBuffer.push(__Internal_BTSerial.read());
    } else this->error = ErrorCode::READ_BUFFER_NULL;
}

#endif