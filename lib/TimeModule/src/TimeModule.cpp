#include "TimeModule.h"
#include "Logger.h"
#include <ESP8266WiFi.h>
#include <time.h>

TimeModule::TimeModule() : settings(nullptr), rtcFound(false) {}

void TimeModule::begin(TerminalCLI& cli, SettingsModule& s) {
    settings = &s;
    // Initialize I2C with the configured pins
    Wire.begin(Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);
    
    // Initialize DS1307
    if (!rtc.begin()) {
        Logger::error("Couldn't find RTC DS1307");
        rtcFound = false;
    } else {
        rtcFound = true;
        Logger::info("RTC DS1307 initialized successfully");
        if (!rtc.isrunning()) {
            Logger::warn("RTC is NOT running, let's set the time!");
        }
    }

    // Register CLI Commands
    cli.registerCommand("time", "Display current RTC time", 
        [this](String args) { this->handleTimeCommand(args); });
    cli.registerCommand("sync", "Sync RTC time with NTP Server via WiFi", 
        [this](String args) { this->handleSyncCommand(args); });

    // Automatically try to sync at startup
    syncWithNTP();
}

void TimeModule::update() {
    // Empty for now, but could be used to occasionally re-sync
}

void TimeModule::syncWithNTP() {
    Logger::info("Connecting to WiFi for NTP sync...");
    Logger::debug("SSID: ");
    Logger::debug(settings->getSSID());

    // Start WiFi connection
    WiFi.begin(settings->getSSID(), settings->getPassword());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Logger::raw(".");
        attempts++;
    }
    Logger::rawln();

    if (WiFi.status() != WL_CONNECTED) {
        Logger::error("Failed to connect to WiFi. NTP sync aborted.");
        return;
    }

    Logger::info("WiFi connected. IP: ");
    Logger::info(WiFi.localIP().toString().c_str());
    
    // Setup NTP
    Logger::info("Requesting time from NTP server...");
    configTime(Config::GMT_OFFSET_SEC, Config::DAYLIGHT_OFFSET_SEC, settings->getNtpServer());

    // Wait for time to be set
    time_t now = time(nullptr);
    attempts = 0;
    while (now < 24 * 3600 && attempts < 20) {
        delay(500);
        Logger::raw(".");
        now = time(nullptr);
        attempts++;
    }
    Logger::rawln();

    if (now < 24 * 3600) {
        Logger::error("Failed to get time from NTP server.");
    } else {
        struct tm timeinfo;
        gmtime_r(&now, &timeinfo);
        
        // Output fetched time
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        Logger::info("NTP Time acquired: ");
        Logger::info(timeStr);

        // Update RTC if it exists
        if (rtcFound) {
            rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, 
                                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
            Logger::info("RTC successfully updated from NTP!");
        } else {
            Logger::warn("NTP time fetched, but no RTC to update.");
        }
    }

    // Disconnect WiFi to save power/resources since we only need it for sync
    WiFi.disconnect(true);
    Logger::info("WiFi disconnected.");
}

void TimeModule::handleTimeCommand(String args) {
    if (!rtcFound) {
        Logger::error("RTC module not found!");
        return;
    }

    DateTime now = rtc.now();
    
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
             
    Logger::rawln("Current RTC Time:");
    Logger::rawln(buf);
}

void TimeModule::handleSyncCommand(String args) {
    syncWithNTP();
}
