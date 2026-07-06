#pragma once

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "ScaleDriver.h"
#include "GasSensorModule.h"
#include "SettingsModule.h"
#include "TimeModule.h"

class TelegramModule {
public:
    TelegramModule(ScaleDriver& scale, GasSensorModule& gasSensor, TimeModule& time);
    
    void begin(SettingsModule& settings);
    void update();

    void sendMessage(const String& msg);
    bool hasNewMessage();

    String getLastMessage();
    String getLastChatId();

private:
    void sendMessageToChat(const String& chatId, const String& msg);
    float getGasPercentage();

    ScaleDriver& _scale;
    GasSensorModule& _gasSensor;
    TimeModule& _time;
    SettingsModule* _settings;

    WiFiClientSecure client;
    UniversalTelegramBot* bot;

    bool _leakAlertSent = false;
    bool _lowGasAlertSent = false;
    unsigned long _lastUpdateMs = 0;

    String lastMessage = "";
    String lastChatId = "";
};