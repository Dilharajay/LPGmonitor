#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Logger.h"

class WiFiWorker
{
public:
    void begin(const char* ssid, const char* password);

    bool isConnected();
    bool connect(uint32_t timeoutMs = 15000, void (*tickCb)() = nullptr);

    void update();   // FIX: allows auto-reconnect
    IPAddress ip();

private:
    const char* _ssid = nullptr;
    const char* _password = nullptr;

    bool _lastState = false;
    uint32_t _lastReconnectAttempt = 0;
};