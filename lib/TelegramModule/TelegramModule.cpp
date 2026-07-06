#include "TelegramModule.h"
#include "Config.h"
#include "Logger.h"

void TelegramModule::begin() {
    client.setInsecure();  // production: use CA cert instead

    bot = new UniversalTelegramBot(Config::TELEGRAM_BOT_TOKEN, client);
}

void TelegramModule::sendMessage(const String& msg) {
    if (!bot) {
        Logger::error(F("Telegram bot not initialized. Message skipped."));
        return;
    }

    bot->sendMessage(Config::TELEGRAM_CHAT_ID, msg, "");
}

void TelegramModule::sendMessageToChat(const String& chatId, const String& msg) {
    if (!bot) {
        Logger::error(F("Telegram bot not initialized. Message skipped."));
        return;
    }

    bot->sendMessage(chatId, msg, "");
}

void TelegramModule::update() {

    int numNewMessages =
        bot->getUpdates(bot->last_message_received + 1);

    while (numNewMessages) {

        for (int i = 0; i < numNewMessages; i++) {

            lastMessage = bot->messages[i].text;
            lastChatId = bot->messages[i].chat_id;

            Logger::info(F("Received message: ") + lastMessage);

            // COMMANDS
            if (lastMessage == "/start") {
                sendMessageToChat(lastChatId, "ESP32 Bot Online");
            }

            else if (lastMessage == "/status") {
                sendMessageToChat(lastChatId, "System OK");
            }

            else if (lastMessage == "/ping") {
                sendMessageToChat(lastChatId, "pong");
            }

            else if (lastMessage == "/help") {
                sendMessageToChat(
                    lastChatId,
                    "/status\n/ping\n/help"
                );
            }
        }

        numNewMessages =
            bot->getUpdates(bot->last_message_received + 1);
    }
}

String TelegramModule::getLastMessage() {
    return lastMessage;
}

String TelegramModule::getLastChatId() {
    return lastChatId;
}

bool TelegramModule::hasNewMessage() {
    return lastMessage.length() > 0;
}