#include "TelegramModule.h"
#include "Config.h"
#include "Logger.h"

TelegramModule::TelegramModule(ScaleDriver& scale, GasSensorModule& gasSensor, TimeModule& time)
    : _scale(scale), _gasSensor(gasSensor), _time(time), _settings(nullptr), bot(nullptr) {}

void TelegramModule::begin(SettingsModule& settings) {
    _settings = &settings;
    client.setInsecure();  // production: use CA cert instead
    client.setBufferSizes(1024, 1024); // Reduce BearSSL memory footprint
    client.setTimeout(5000); // Prevent blocking indefinitely
    
    if (String(Config::TELEGRAM_BOT_TOKEN).length() > 0) {
        bot = new UniversalTelegramBot(Config::TELEGRAM_BOT_TOKEN, client);
    } else {
        Logger::warn(F("No Telegram bot token configured. Bot disabled."));
    }
}

float TelegramModule::getGasPercentage() {
    if (!_settings) return 0.0f;
    float weight = _scale.getFilteredWeight();
    float emptyWeight = _settings->getEmptyCylinderWeight();
    float fullWeight = _settings->getFullCylinderWeight();
    float gasCapacity = fullWeight - emptyWeight;
    
    if (gasCapacity <= 0.0f) return 0.0f;
    float gasRemaining = weight - emptyWeight;
    float gasPercent = (gasRemaining / gasCapacity) * 100.0f;
    if (gasPercent < 0.0f) gasPercent = 0.0f;
    if (gasPercent > 100.0f) gasPercent = 100.0f;
    return gasPercent;
}

void TelegramModule::sendMessage(const String& msg) {
    if (!bot) return;
    if (String(Config::TELEGRAM_CHAT_ID).length() > 0) {
        bot->sendMessage(Config::TELEGRAM_CHAT_ID, msg, "");
    }
}

void TelegramModule::sendMessageToChat(const String& chatId, const String& msg) {
    if (!bot) return;
    bot->sendMessage(chatId, msg, "");
}

void TelegramModule::update() {
    if (!bot || !_settings) return;

    unsigned long now = millis();
    
    // Check for alerts (rate limit to check every 1 second)
    if (now - _lastUpdateMs > 1000) {
        _lastUpdateMs = now;
        
        // 1. Gas Leak Alert
        if (_gasSensor.isLeakDetected()) {
            if (!_leakAlertSent) {
                String alert = "🚨 <b>GAS LEAK DETECTED!</b> 🚨\n\n";
                alert += "Sensor Reading: " + String(_gasSensor.getPPM()) + " ppm\n";
                alert += "Time: " + _time.getTimeString() + "\n";
                alert += "Please check your cylinder immediately!";
                if (String(Config::TELEGRAM_CHAT_ID).length() > 0) {
                    bot->sendMessage(Config::TELEGRAM_CHAT_ID, alert, "HTML");
                }
                _leakAlertSent = true;
            }
        } else {
            _leakAlertSent = false;
        }

        // 2. Low Gas Alert
        float gasLevel = getGasPercentage();
        if (gasLevel < 10.0f) {
            if (!_lowGasAlertSent && gasLevel > 0.0f) {
                String alert = "⚠️ <b>Low Gas Warning</b> ⚠️\n\n";
                alert += "Gas level is running low (" + String(gasLevel, 1) + "%).\n";
                alert += "Time to order a new cylinder!";
                if (String(Config::TELEGRAM_CHAT_ID).length() > 0) {
                    bot->sendMessage(Config::TELEGRAM_CHAT_ID, alert, "HTML");
                }
                _lowGasAlertSent = true;
            }
        } else {
            // Reset if above 15% (hysteresis)
            if (gasLevel > 15.0f) {
                _lowGasAlertSent = false;
            }
        }
    }

    // Handle incoming messages - rate limit to check every 3 seconds to prevent HTTPS OOM crashes
    static unsigned long _lastTelegramPollMs = 0;
    if (now - _lastTelegramPollMs > 3000) {
        _lastTelegramPollMs = now;
        
        int numNewMessages = bot->getUpdates(bot->last_message_received + 1);
        while (numNewMessages) {
            for (int i = 0; i < numNewMessages; i++) {
                lastMessage = bot->messages[i].text;
                lastChatId = bot->messages[i].chat_id;

                Logger::info(F("Received message: ") + lastMessage);

                if (lastMessage == "/start") {
                    sendMessageToChat(lastChatId, "ESP8266 Gas Monitor Bot Online.\nSend /help for commands.");
                }
                else if (lastMessage == "/status") {
                    String msg = "📊 <b>System Status</b>\n\n";
                    msg += "Time: " + _time.getTimeString() + "\n";
                    msg += "WiFi: Connected\n";
                    msg += "Leak Status: " + String(_gasSensor.isLeakDetected() ? "LEAK" : "Normal") + "\n";
                    bot->sendMessage(lastChatId, msg, "HTML");
                }
                else if (lastMessage == "/gaslevel") {
                    float level = getGasPercentage();
                    float weight = _scale.getFilteredWeight();
                    String msg = "🛢️ <b>Gas Level</b>\n\n";
                    msg += "Remaining: " + String(level, 1) + "%\n";
                    msg += "Current Weight: " + String(weight / 1000.0f, 3) + " kg\n";
                    bot->sendMessage(lastChatId, msg, "HTML");
                }
                else if (lastMessage == "/ping") {
                    sendMessageToChat(lastChatId, "pong");
                }
                else if (lastMessage == "/help") {
                    String msg = "Available commands:\n";
                    msg += "/status - Check system status\n";
                    msg += "/gaslevel - Check remaining gas percentage\n";
                    msg += "/ping - Check if bot is responsive\n";
                    msg += "/help - Show this menu\n";
                    sendMessageToChat(lastChatId, msg);
                }
            }
            numNewMessages = bot->getUpdates(bot->last_message_received + 1);
        }
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