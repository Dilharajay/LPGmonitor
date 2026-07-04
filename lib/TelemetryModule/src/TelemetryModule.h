#ifndef TELEMETRY_MODULE_H
#define TELEMETRY_MODULE_H

#include <Arduino.h>
#include "TerminalCLI.h"
#include "SettingsModule.h"
#include "ScaleDriver.h"
#include "TimeModule.h"

class TelemetryModule {
public:
    TelemetryModule(ScaleDriver& scaleDriver, TimeModule& timeMod);
    
    // Begins the telemetry loop. If telemetry is enabled, it blocks, runs telemetry, and sleeps.
    // If not enabled, it returns immediately.
    void begin(TerminalCLI& cli, SettingsModule& s);
    void update();

private:
    SettingsModule* settings;
    ScaleDriver& scale;
    TimeModule& timeModule;
    unsigned long lastPostTime;
    void postData();
};

#endif
