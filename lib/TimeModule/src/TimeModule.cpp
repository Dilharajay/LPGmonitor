#include "TimeModule.h"
#include "Logger.h"
#include <ESP8266WiFi.h>
#include <time.h>

TimeModule::TimeModule() : settings(nullptr), rtcFound(false) {}

void TimeModule::begin(TerminalCLI& cli, SettingsModule& s) {
    settings = &s;
    // I2C is now initialized globally in main.cpp
    
    // Initialize DS1307
    if (!rtc.begin()) {
        Logger::error(F("Couldn't find RTC DS1307"));
        rtcFound = false;
    } else {
        rtcFound = true;
        Logger::info(F("RTC DS1307 initialized successfully"));
        if (!rtc.isrunning()) {
            Logger::warn(F("RTC is NOT running, let's set the time!"));
        }
    }

    // Register CLI Commands
    cli.registerCommand("time", "Display current RTC time", 
        [this](String args) { this->handleTimeCommand(args); });
    cli.registerCommand("sync", "Sync RTC time with NTP Server via WiFi", 
        [this](String args) { this->handleSyncCommand(args); });

    // Automatically try to sync at startup if needed
    bool needsSync = false;
    if (!rtcFound) {
        needsSync = true;
    } else if (!rtc.isrunning()) {
        needsSync = true;
    } else {
        DateTime now = rtc.now();
        if (now.year() < 2024) {
            needsSync = true;
        }
    }

    if (needsSync) {
        Logger::info(F("Time is not set. Syncing with NTP..."));
        syncWithNTP();
    } else {
        Logger::info(F("RTC time looks valid. Skipping NTP sync."));
    }
}

static bool ntpSyncInProgress = false;
static unsigned long ntpSyncStartMs = 0;
static int ntpAttempts = 0;

void TimeModule::update() {
    if (ntpSyncInProgress) {
        if (millis() - ntpSyncStartMs > 500) {
            ntpSyncStartMs = millis();
            time_t now = time(nullptr);
            if (now > 24 * 3600) {
                ntpSyncInProgress = false;
                struct tm timeinfo;
                localtime_r(&now, &timeinfo); // Use local time for RTC
                
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
            } else {
                ntpAttempts++;
                if (ntpAttempts >= 40) {
                    ntpSyncInProgress = false;
                    Logger::error(F("Failed to get time from NTP server."));
                }
            }
        }
    }
}

void TimeModule::syncWithNTP() {
    Logger::info(F("Requesting time from NTP server..."));
    configTime(settings->getTimezoneOffsetSec(), Config::DAYLIGHT_OFFSET_SEC, settings->getNtpServer());
    ntpSyncInProgress = true;
    ntpSyncStartMs = millis();
    ntpAttempts = 0;
}

String TimeModule::getTimeString() {
    if (!rtcFound) {
        time_t now_t = time(nullptr);
        if (now_t < 24 * 3600) {
            return "1970-01-01 00:00:00"; // Not synced yet
        }
        struct tm timeinfo;
        localtime_r(&now_t, &timeinfo);
        char buf[32];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                 timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        return String(buf);
    }
    
    DateTime now = rtc.now();
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
    return String(buf);
}

void TimeModule::handleTimeCommand(String args) {
    Logger::raw("Current Time: ");
    Logger::rawln(getTimeString().c_str());
    if (!rtcFound) Logger::warn(F("(No RTC — using NTP software clock)"));
}

void TimeModule::handleSyncCommand(String args) {
    syncWithNTP();
}
