#pragma once

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

class TelegramModule {
public:
    void begin();
    void update();

    void sendMessage(const String& msg);
    bool hasNewMessage();

    String getLastMessage();
    String getLastChatId();

private:
    void sendMessageToChat(const String& chatId, const String& msg);

    WiFiClientSecure client;
    UniversalTelegramBot* bot;

    int lastUpdateTime = 0;

    String lastMessage = "";
    String lastChatId = "";
};