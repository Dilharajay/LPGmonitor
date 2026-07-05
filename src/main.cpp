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
#include "OTA/OTAmanager.h"
#include "LEDModule.h"

// ── Global Modules ─────────────────────────────────────────────────
TerminalCLI cli;
ScaleDriver scaleDriver;
ScaleModule scaleModule(scaleDriver);
GasSensorModule gasSensor;
SettingsModule settingsModule;
TimeModule  timeModule;
WebInterfaceModule webModule(scaleDriver, gasSensor, timeModule);
MqttModule mqttModule(scaleDriver, gasSensor, timeModule);
LEDModule ledModule;
bool wifiLogged = false;

void setup()
{
    Serial.begin(Config::SERIAL_BAUD_RATE);
    
    // 1. Initialize Logger
    Logger::begin(false); // Debug OFF by default
    
    // 2. Initialize Config and EEPROM (do this before hardware so we can read settings)
    settingsModule.begin(cli);

    // 3. Start WiFi (non-blocking)
    Logger::info(F("Starting WiFi (non-blocking)..."));
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(settingsModule.getSSID(), settingsModule.getPassword());

    // 4. Initialize Hardware Drivers
    if (!scaleDriver.begin(Config::HX711_DOUT_PIN, Config::HX711_SCK_PIN, Config::DEFAULT_CALIBRATION_FACTOR, settingsModule.getTareOffset())) {
        Logger::error(F("Scale initialization failed. Continuing in degraded mode."));
    }
    
    // 4.5. Initialize LED Status Indicator
    ledModule.begin();
    ledModule.setMode(LEDMode::CONNECTING);
    
    // 5. Initialize Gas Sensor
    gasSensor.begin(cli);
    gasSensor.setLeakThreshold(settingsModule.getGasLeakThreshold());
    
    // 6. Register Global Commands to CLI
    cli.registerCommand("debug", "Toggle debug logging output", [](String args) {
        bool newMode = !Logger::isDebugMode();
        Logger::setDebugMode(newMode);
        Logger::info(newMode ? F("Debug logging: ON") : F("Debug logging: OFF"));
    });
    
    // 7. Initialize and Register Feature Modules
    scaleModule.begin(cli, settingsModule);
    timeModule.begin(cli, settingsModule);
    mqttModule.begin(settingsModule);
    
    // 8. Start CLI (prints welcome prompt and help)
    cli.begin("\n=== Smart LPG Monitor Ready ===");

    // 9. Start Web Server Interface (if enabled)
    if (settingsModule.isWebInterfaceEnabled()) {
        Logger::info(F("Web Interface is ENABLED. Starting web server..."));
        webModule.begin(settingsModule, cli);
    } else {
        Logger::info(F("Web Interface is DISABLED."));
    }
    
    cli.printHelp();

    // 10. Initialize OTA Updates
    OTA::begin(
        Config::OTA_HOSTNAME,
        settingsModule.getOtaPassword()
    );
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

    // Update LED status based on WiFi and streaming state
    if (scaleModule.isStreaming()) {
        ledModule.setStreaming(true);
    } else {
        ledModule.setStreaming(false);
    }
    ledModule.update();

    // Log WiFi connection once when established
    if (!wifiLogged && WiFi.status() == WL_CONNECTED) {
        wifiLogged = true;
        Logger::info(F("WiFi connected. IP: "));
        Logger::info(WiFi.localIP().toString().c_str());
    }
    
    if (settingsModule.isWebInterfaceEnabled()) {
        webModule.update();
    }
    
    // Yield to ESP8266 background tasks
    delay(10);

    // Handle OTA updates
    OTA::loop();
}