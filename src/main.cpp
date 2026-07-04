#include <Arduino.h>
#include "TerminalCLI.h"
#include "ScaleDriver.h"
#include "ScaleModule.h"
#include "Logger.h"
#include "Config.h"
#include "SettingsModule.h"
#include "TimeModule.h"

// ── Global Modules ─────────────────────────────────────────────────
TerminalCLI cli;
ScaleDriver scaleDriver;
ScaleModule scaleModule(scaleDriver);
SettingsModule settingsModule;
TimeModule  timeModule;

void setup()
{
    Serial.begin(Config::SERIAL_BAUD_RATE);
    
    // 1. Initialize Logger
    Logger::begin(false); // Debug OFF by default
    
    // 2. Initialize Hardware Drivers
    scaleDriver.begin(Config::HX711_DOUT_PIN, Config::HX711_SCK_PIN, Config::DEFAULT_CALIBRATION_FACTOR);
    
    // 3. Register Global Commands to CLI
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
    
    // 4. Initialize and Register Feature Modules
    settingsModule.begin(cli);
    scaleModule.begin(cli);
    timeModule.begin(cli, settingsModule);
    
    // 5. Start CLI (prints welcome prompt and help)
    cli.begin("\n=== System Ready ===");
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