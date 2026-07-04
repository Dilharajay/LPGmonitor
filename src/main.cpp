#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "TerminalCLI.h"
#include "ScaleDriver.h"
#include "ScaleModule.h"
#include "GasSensorModule.h"
#include "Logger.h"
#include "Config.h"
#include "SettingsModule.h"
#include "TimeModule.h"
#include "WebInterfaceModule.h"
#include "MqttModule.h"

// ── Global Modules ─────────────────────────────────────────────────
TerminalCLI cli;
ScaleDriver scaleDriver;
ScaleModule scaleModule(scaleDriver);
GasSensorModule gasSensor;
SettingsModule settingsModule;
TimeModule  timeModule;
WebInterfaceModule webModule(scaleDriver, gasSensor, timeModule);
MqttModule mqttModule(scaleDriver, gasSensor, timeModule);

void setup()
{
    Serial.begin(Config::SERIAL_BAUD_RATE);
    
    // 1. Initialize Logger
    Logger::begin(false); // Debug OFF by default
    
    // 2. Initialize Config and EEPROM (do this before hardware so we can read settings)
    settingsModule.begin(cli);

    // 3. Connect to WiFi
    Logger::info("Connecting to WiFi...");
    WiFi.begin(settingsModule.getSSID(), settingsModule.getPassword());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Logger::raw(".");
        attempts++;
    }
    Logger::rawln();
    if (WiFi.status() == WL_CONNECTED) {
        Logger::info("WiFi connected. IP: ");
        Logger::info(WiFi.localIP().toString().c_str());
    } else {
        Logger::error("WiFi connection failed.");
    }

    // 4. Initialize Hardware Drivers
    scaleDriver.begin(Config::HX711_DOUT_PIN, Config::HX711_SCK_PIN, Config::DEFAULT_CALIBRATION_FACTOR, settingsModule.getTareOffset());
    
    // 5. Initialize Gas Sensor
    gasSensor.begin(cli);
    gasSensor.setLeakThreshold(settingsModule.getGasLeakThreshold());
    
    // 6. Register Global Commands to CLI
    cli.registerCommand("debug", "Toggle debug logging output", [](String args) {
        bool newMode = !Logger::isDebugMode();
        Logger::setDebugMode(newMode);
        Logger::info(newMode ? "Debug logging: ON" : "Debug logging: OFF");
    });
    
    // 7. Initialize and Register Feature Modules
    scaleModule.begin(cli, settingsModule);
    timeModule.begin(cli, settingsModule);
    mqttModule.begin(settingsModule);
    
    // 8. Start CLI (prints welcome prompt and help)
    cli.begin("\n=== Smart LPG Monitor Ready ===");

    // 9. Start Web Server Interface (if enabled)
    if (settingsModule.isWebInterfaceEnabled()) {
        Logger::info("Web Interface is ENABLED. Starting web server...");
        webModule.begin(settingsModule, cli);
    } else {
        Logger::info("Web Interface is DISABLED.");
    }
    
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
    mqttModule.update();
    
    if (settingsModule.isWebInterfaceEnabled()) {
        webModule.update();
    }
    
    // Yield to ESP8266 background tasks
    delay(10);
}