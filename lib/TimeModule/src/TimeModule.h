#ifndef TIME_MODULE_H
#define TIME_MODULE_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include "TerminalCLI.h"
#include "Config.h"
#include "SettingsModule.h"

class TimeModule {
public:
    TimeModule();
    void begin(TerminalCLI& cli, SettingsModule& settings);
    void update();

private:
    RTC_DS1307 rtc;
    SettingsModule* settings;
    bool rtcFound;

    void handleTimeCommand(String args);
    void handleSyncCommand(String args);
    void syncWithNTP();
};

#endif // TIME_MODULE_H
