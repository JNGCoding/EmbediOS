#include "WirelessConnectivity.hpp"

#ifdef COMPILE_WITH_WIFI_INITIALIZATION

EmbediWiFiMode EmbediWiFiManager::mode = EmbediWiFiMode::UNACTIVE;
TypeMacros::u64 EmbediWiFiClient::counter = 0;
WiFiClient acceptedClient;

EmbediWiFiClient::EmbediWiFiClient()
{
    this->id = ++EmbediWiFiClient::counter;
    snprintf(this->name, 64, "WiFiClient_%llu", EmbediWiFiClient::counter);

    this->descriptor = {
        this->name,
        "WiFi/Server/",
        READ | WRITE
    };
}

EmbediWiFiClient::EmbediWiFiClient(WiFiClient* cli)
{
    this->client = cli;
    this->id = ++EmbediWiFiClient::counter;
    snprintf(this->name, 64, "WiFiClient_%llu", EmbediWiFiClient::counter);

    this->descriptor = {
        this->name,
        "WiFiServer//",
        READ | WRITE
    };
}


EmbediWiFiClient::~EmbediWiFiClient()
{
    this->end();
}

bool EmbediWiFiClient::set_client(WiFiClient* c)
{
    if (c == nullptr)
        return false;

    this->client = c;
    return true;
}

bool EmbediWiFiClient::connect(const char* host, const int port)
{
    if (host == nullptr || this->client == nullptr)
        return false;

    return this->client->connect(host, port);
}

void EmbediWiFiClient::end()
{
    if (this->client != nullptr)
        this->client->stop();

    this->client = nullptr;
}

bool EmbediWiFiClient::write(const TypeMacros::u8 _byte)
{
    if (this->client == nullptr)
        return false;

    return this->client->write(_byte) == 1;
}

TypeMacros::u8 EmbediWiFiClient::read()
{
    if (this->client == nullptr)
        return 0;

    return this->client->read();
}

TypeMacros::u32 EmbediWiFiClient::available()
{
    if (this->client == nullptr)
        return 0;

    return this->client->available();
}

bool EmbediWiFiClient::isnull()
{
    return this->client == nullptr;
}

void EmbediWiFiClient::internalreset()
{
    if (this->isnull())
        return;

    this->client->stop();
    this->client = nullptr;
}

void EmbediWiFiClient::flush()
{
    if (this->client != nullptr)
        this->client->flush();
}

bool EmbediWiFiClient::connected()
{
    if (this->client != nullptr)
        return this->client->connected();

    return false;
}

EmbediWiFiServer::EmbediWiFiServer(WiFiServer& serv) : server(serv)
{
}

EmbediWiFiServer::~EmbediWiFiServer()
{
    if (this->serverOpened)
        this->server.end();
}

bool EmbediWiFiServer::accept(EmbediWiFiClient& client)
{
    if (!this->server.hasClient())
        return false;

    WiFiClient nextClient = this->server.accept();
    if (!nextClient)
        return false;

    acceptedClient = nextClient;
    return client.set_client(&acceptedClient);

}

bool EmbediWiFiServer::clients_waiting()
{
    return this->server.hasClient();
}

void EmbediWiFiServer::start()
{
    if (!this->serverOpened)
    {
        this->server.begin();
        this->server.setNoDelay(true);
        this->serverOpened = true;
    }
}

void EmbediWiFiServer::end()
{
    if (this->serverOpened)
    {
        this->server.end();
        this->serverOpened = false;
    }
}

bool EmbediWiFiServer::opened()
{
    return this->serverOpened;
}

#endif
