#include <Arduino.h>
#include "TerminalCLI.h"
#include "ScaleDriver.h"
#include "ScaleModule.h"
#include "Logger.h"
#include "Config.h"
#include "SettingsModule.h"
#include "TimeModule.h"
#include "TelemetryModule.h"

// ── Global Modules ─────────────────────────────────────────────────
TerminalCLI cli;
ScaleDriver scaleDriver;
ScaleModule scaleModule(scaleDriver);
SettingsModule settingsModule;
TimeModule  timeModule;
TelemetryModule telemetryModule(scaleDriver, timeModule);

void setup()
{
    Serial.begin(Config::SERIAL_BAUD_RATE);
    
    // 1. Initialize Logger
    Logger::begin(false); // Debug OFF by default
    
    // 2. Initialize Config and EEPROM (do this before hardware so we can read settings)
    settingsModule.begin(cli);

    // 3. Initialize Hardware Drivers
    scaleDriver.begin(Config::HX711_DOUT_PIN, Config::HX711_SCK_PIN, Config::DEFAULT_CALIBRATION_FACTOR, settingsModule.getTareOffset());
    
    // 4. Register Global Commands to CLI
    cli.registerCommand("d", "Toggle debug logging output", [](String args) {
        bool newMode = !Logger::isDebugMode();
        Logger::setDebugMode(newMode);
        Logger::info(newMode ? "Debug logging: ON" : "Debug logging: OFF");
    });
    cli.registerCommand("debug", "Toggle debug logging output", [](String args) {
        bool newMode = !Logger::isDebugMode();
        Logger::setDebugMode(newMode);
        Logger::info(newMode ? "Debug logging: ON" : "Debug logging: OFF");
    });
    
    // 5. Initialize and Register Feature Modules
    scaleModule.begin(cli, settingsModule);
    timeModule.begin(cli, settingsModule);
    
    // 6. Start CLI (prints welcome prompt and help)
    cli.begin("\n=== System Ready ===");

    // 7. Telemetry Mode Check (blocks and deep sleeps if enabled)
    telemetryModule.begin(cli, settingsModule);
    cli.printHelp();
}

void loop()
{
    // Process serial input for commands
    cli.handle();
    
    // Update feature modules (e.g. read sensors, stream data)
    scaleModule.update();
    timeModule.update();
    
    // Yield to ESP8266 background tasks
    delay(10);
}