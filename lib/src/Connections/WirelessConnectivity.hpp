#ifndef __EMBEDI_WIFI_CONNECTIVITY_HPP__
#define __EMBEDI_WIFI_CONNECTIVITY_HPP__

#include <Arduino.h>
#include <string.h>

#include "TypeMacros.hpp"
#include "Defines.hpp"
#include "IOStreams.hpp"

#include "Allocator.hpp"
#include "DataStructures.hpp"

#ifdef COMPILE_WITH_WIFI_INITIALIZATION

#include <WiFi.h>

struct EmbediIPAddress
{
    TypeMacros::u8 a, b, c, d;

    EmbediIPAddress()
    {
        this->a = 0;
        this->b = 0;
        this->c = 0;
        this->d = 0;
    }

    EmbediIPAddress(TypeMacros::u32 address)
    {
        this->a = static_cast<TypeMacros::u8>(address >> 24);
        this->b = static_cast<TypeMacros::u8>(address >> 16);
        this->c = static_cast<TypeMacros::u8>(address >> 8);
        this->d = static_cast<TypeMacros::u8>(address & 0xFF);
    }

    EmbediIPAddress(TypeMacros::u8 _a, TypeMacros::u8 _b, TypeMacros::u8 _c, TypeMacros::u8 _d)
    {
        this->a = _a;
        this->b = _b;
        this->c = _c;
        this->d = _d;
    }

    TypeMacros::u8 get_octet(TypeMacros::u8 index)
    {
        if (index == 0)
        {
            return this->a;
        }
        else if (index == 1)
        {
            return this->b;
        }
        else if (index == 2)
        {
            return this->c;
        }
        else if (index == 3)
        {
            return this->d;
        }
        else return 0;
    }

    void to_string(char* buffer)
    {
        sprintf(buffer, "%u.%u.%u.%u", this->a, this->b, this->c, this->d);
    }
};

enum class EmbediWiFiMode
{
    UNACTIVE,
    WIFI,
    SOFTAP,
};

struct EmbediWiFiConfig
{
    char SSID[64];
    char PASS[64];
    EmbediWiFiMode mode;
};

class EmbediWiFiManager
{
public:
    static EmbediWiFiMode mode;

    static bool init_wifi(EmbediWiFiConfig config, const TypeMacros::u32 timeout = 10000UL)
    {
        if (config.mode == EmbediWiFiMode::WIFI)
        {
            WiFi.mode(WIFI_STA);
            WiFi.begin(config.SSID, config.PASS);

            TypeMacros::u32 start = millis();
            while (WiFi.status() != WL_CONNECTED)
            {
                if (millis() - start >= timeout)
                {
                    WiFi.disconnect(true, true);
                    return false;
                }
            }

            EmbediWiFiManager::mode = EmbediWiFiMode::WIFI;

            return true;
        }
        else
        {
            WiFi.mode(WIFI_AP);
            if (!WiFi.softAP(config.SSID, config.PASS))
                return false;

            EmbediWiFiManager::mode = EmbediWiFiMode::SOFTAP;
            return true;
        }

        return false;
    }

    static void deinit_wifi()
    {
        switch (EmbediWiFiManager::mode)
        {
            case EmbediWiFiMode::WIFI:
                WiFi.disconnect(true, true);
                break;

            case EmbediWiFiMode::SOFTAP:
                WiFi.softAPdisconnect(true);
                break;

            default:
                return;
        }
    }

    static void get_ip(EmbediIPAddress& ip)
    {
        switch (EmbediWiFiManager::mode)
        {
            case EmbediWiFiMode::WIFI: {
                IPAddress addr = WiFi.localIP();
                ip.a = addr[0];
                ip.b = addr[1];
                ip.c = addr[2];
                ip.d = addr[3];
            } break;

            case EmbediWiFiMode::SOFTAP: {
                IPAddress addr = WiFi.softAPIP();
                ip.a = addr[0];
                ip.b = addr[1];
                ip.c = addr[2];
                ip.d = addr[3];
            } break;

            default:
                return;
        }
    }
};

class EmbediWiFiClient : public EmbediFileStream
{
private:
    WiFiClient* client = nullptr;
    static TypeMacros::u64 counter;
    TypeMacros::u64 id = 0;
    char name[64];

public:
    EmbediWiFiClient();
    EmbediWiFiClient(WiFiClient* cli);
    ~EmbediWiFiClient();

    bool set_client(WiFiClient* c);

    bool write(const TypeMacros::u8 _byte) override;

    TypeMacros::u8 read() override;
    TypeMacros::u32 available() override;

    bool connect(const char* host, const int port);
    void end();

    void flush() override;

    bool isnull();
    void internalreset();

    bool connected();
};

class EmbediWiFiServer
{
private:
    WiFiServer& server;
    bool serverOpened = false;

public:
    EmbediWiFiServer(WiFiServer& serv);
    ~EmbediWiFiServer();

    bool accept(EmbediWiFiClient& client);
    bool clients_waiting();

    void start();
    void end();

    bool opened();
};

#endif

#ifdef COMPILE_WITH_BLUETOOTH_INITIALIZATION

#include <BluetoothSerial.h>

struct EmbediBluetoothConfig
{
    char name[64];
};

extern BluetoothSerial __Internal_BTSerial;
extern EmbediBluetoothConfig __Internal_config;
extern bool __Internal_BluetoothActivated;

struct EmbediBluetoothManager
{
    static void init_bluetooth(EmbediBluetoothConfig config)
    {
        __Internal_BTSerial.begin(config.name);
        __Internal_BluetoothActivated = true;
        memcpy(__Internal_config.name, config.name, 64 * sizeof(char));
    }

    static void deinit_bluetooth()
    {
        __Internal_BTSerial.end();
        __Internal_BluetoothActivated = false;
        memset(__Internal_config.name, 0, 64 * sizeof(char));
    }
};

struct EmbediBluetoothSerialOutput : public EmbediFileStream
{
    EmbediBluetoothSerialOutput();
    ~EmbediBluetoothSerialOutput();

    bool write(const TypeMacros::u8 _byte) override;
    void flush() override;
};

struct EmbediBluetoothSerialInput : public EmbediFileStream
{
    EmbediBluetoothSerialInput();

    TypeMacros::u8 read() override;
    TypeMacros::u32 available() override;
};

#endif

#endif