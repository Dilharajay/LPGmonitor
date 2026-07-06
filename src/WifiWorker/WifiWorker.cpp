#include "WiFiWorker.h"

void WiFiWorker::begin(const char* ssid, const char* password)
{
    _ssid = ssid;
    _password = password;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Logger::info("WiFi module initialized");
}

bool WiFiWorker::connect(uint32_t timeoutMs)
{
    if (!_ssid || !_password)
    {
        Logger::error("WiFi credentials not set");
        return false;
    }

    Logger::info("Connecting WiFi...");

    WiFi.begin(_ssid, _password);

    uint32_t start = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        yield(); // FIX: watchdog safety

        if (millis() - start > timeoutMs)
        {
            Logger::error("WiFi connect timeout");
            return false;
        }
    }

    Logger::info("WiFi connected");
    Logger::info(WiFi.localIP().toString().c_str());

    return true;
}

bool WiFiWorker::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WiFiWorker::ip()
{
    return WiFi.localIP();
}

void WiFiWorker::update()
{
    bool connected = isConnected();

    // FIX: detect state change
    if (connected && !_lastState)
    {
        Logger::info("WiFi reconnected");
    }

    if (!connected)
    {
        // FIX: non-blocking auto reconnect
        if (millis() - _lastReconnectAttempt > 5000)
        {
            Logger::error("WiFi lost. Reconnecting...");
            WiFi.reconnect();
            _lastReconnectAttempt = millis();
        }
    }

    _lastState = connected;
}