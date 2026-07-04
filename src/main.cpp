#include <Arduino.h>
#include "TerminalCLI.h"
#include "ScaleDriver.h"
#include "ScaleModule.h"
#include "GasSensorModule.h"
#include "Logger.h"
#include "Config.h"
#include "SettingsModule.h"
#include "TimeModule.h"
#include "WebInterfaceModule.h"

// ── Global Modules ─────────────────────────────────────────────────
TerminalCLI cli;
ScaleDriver scaleDriver;
ScaleModule scaleModule(scaleDriver);
GasSensorModule gasSensor;
SettingsModule settingsModule;
TimeModule  timeModule;
WebInterfaceModule webModule(scaleDriver, gasSensor, timeModule);

void setup()
{
    Serial.begin(Config::SERIAL_BAUD_RATE);
    
    // 1. Initialize Logger
    Logger::begin(false); // Debug OFF by default
    
    // 2. Initialize Config and EEPROM (do this before hardware so we can read settings)
    settingsModule.begin(cli);

    // 3. Initialize Hardware Drivers
    scaleDriver.begin(Config::HX711_DOUT_PIN, Config::HX711_SCK_PIN, Config::DEFAULT_CALIBRATION_FACTOR, settingsModule.getTareOffset());
    
    // 4. Initialize Gas Sensor
    gasSensor.begin(cli);
    gasSensor.setLeakThreshold(settingsModule.getGasLeakThreshold());
    
    // 5. Register Global Commands to CLI
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
    
    // 6. Initialize and Register Feature Modules
    scaleModule.begin(cli, settingsModule);
    timeModule.begin(cli, settingsModule);
    
    // 7. Start CLI (prints welcome prompt and help)
    cli.begin("\n=== Smart LPG Monitor Ready ===");

    // 8. Start Web Server Interface
    webModule.begin(settingsModule, cli);
    cli.printHelp();
}

void loop()
{
    // Process serial input for commands
    cli.handle();
    
    // Update feature modules (e.g. read sensors, stream data)
    scaleModule.update();
    gasSensor.update();
    timeModule.update();
    webModule.update();
    
    // Yield to ESP8266 background tasks
    delay(10);
}