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
#include "TelegramModule.h"
#include "WifiWorker/WifiWorker.h"
#include "DisplayModule.h"

// Global Modules 
TerminalCLI cli;
ScaleDriver scaleDriver;
ScaleModule scaleModule(scaleDriver);
GasSensorModule gasSensor;
SettingsModule settingsModule;
TimeModule  timeModule;
TelegramModule telegramModule(scaleDriver, gasSensor, timeModule);

WiFiWorker wifiWorker;
 
// fix: shared context for web and mqtt modules to avoid multiple instances of ScaleDriver, GasSensorModule, and TimeModule
WebInterfaceModule webModule(scaleDriver, gasSensor, timeModule);
MqttModule mqttModule(scaleDriver, gasSensor, timeModule);
LEDModule ledModule;
DisplayModule displayModule(scaleDriver, gasSensor, settingsModule);
bool wifiLogged = false;

void setup()
{
    Serial.begin(Config::SERIAL_BAUD_RATE);
    
    // 1. Initialize Logger
    Logger::begin(false); // Debug OFF by default
    
    // 2. Initialize Config and EEPROM (do this before hardware so we can read settings)
    settingsModule.begin(cli);

    // 3. Initialize LED Module first so it can show connecting status
    Logger::info(F("Initializing LED Module..."));
    ledModule.begin(Config::LED_PIN);

    // 4. Start WiFi (non-blocking)
    wifiWorker.begin(settingsModule.getSSID(), settingsModule.getPassword());
    Logger::info(F("Attempting WiFi connection..."));
    if (wifiWorker.connect(15000, []() { ledModule.update(); })) {
        Logger::info(F("WiFi connected successfully. IP Address: "));
        Logger::info(WiFi.localIP().toString().c_str());

    } else {
        Logger::warn(F("WiFi connection failed or timed out. Continuing in offline mode."));
    }
    
    // 5. Initialize Hardware Drivers
    Logger::info(F("Initializing HX711..."));
    if (!scaleDriver.begin(Config::HX711_DOUT_PIN, Config::HX711_SCK_PIN, Config::DEFAULT_CALIBRATION_FACTOR, settingsModule.getTareOffset())) {
        Logger::error(F("Scale initialization failed. Continuing in degraded mode."));
    }
    
    // 5. Initialize Gas Sensor
    Logger::info(F("Initializing Gas Sensor..."));
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
    timeModule.begin(cli, settingsModule); // Initializes I2C
    mqttModule.begin(settingsModule);
    displayModule.begin();

    // 9. Start CLI (prints welcome prompt and help)
    cli.begin("\n=== Smart LPG Monitor Ready ===");

    // 10. Start Web Server Interface (if enabled)
    if (settingsModule.isWebInterfaceEnabled()) {
        Logger::info(F("Web Interface is ENABLED. Starting web server..."));
        webModule.begin(settingsModule, cli);
    } else {
        Logger::info(F("Web Interface is DISABLED."));
    }

    // 11. Initialize OTA Updates
    Logger::info(F("Initializing OTA updates..."));
    OTA::begin(
        Config::OTA_HOSTNAME,
        settingsModule.getOtaPassword()
    );

    // telegram module initialization
    Logger::info(F("Initializing Telegram Module..."));
    telegramModule.begin(settingsModule);

    cli.printHelp();
}

uint32_t lastWebUpdateMs = 0;

void loop()
{
    // Process serial input for commands
    cli.handle();
    
    // Update feature modules (e.g. read sensors, stream data)
    scaleModule.update();
    gasSensor.update();
    timeModule.update();
    mqttModule.update();
    telegramModule.update();
    displayModule.update();

    // Update LED status based on WiFi and streaming state
    static bool lastStreamingState = false;
    bool currentStreamingState = scaleModule.isStreaming();
    if (currentStreamingState != lastStreamingState) {
        ledModule.setStreaming(currentStreamingState);
        lastStreamingState = currentStreamingState;
    }
    ledModule.update();
    
    if (settingsModule.isWebInterfaceEnabled()) {
        uint32_t now = millis();
        if (now - lastWebUpdateMs >= Config::WEB_UPDATE_INTERVAL_MS) {
            webModule.update();
            lastWebUpdateMs = now;
        }
    }

     // Handle OTA updates
    OTA::loop();

    // Yield to ESP8266 background tasks
    yield();
}