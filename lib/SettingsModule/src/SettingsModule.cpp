#include "SettingsModule.h"
#include "Logger.h"

#define EEPROM_SIZE 512
#define MAGIC_WORD "CFG3"

SettingsModule::SettingsModule() {
    // Defaults will be loaded in begin()
}

void SettingsModule::begin(TerminalCLI& cli) {
    EEPROM.begin(EEPROM_SIZE);
    
    if (isInitialized()) {
        load();
        Logger::info("Settings loaded from EEPROM.");
    } else {
        Logger::warn("EEPROM not initialized. Resetting to defaults.");
        resetToDefaults();
    }

    cli.registerCommand("set_ssid", "Set WiFi SSID (e.g. set_ssid MyNetwork)", 
        [this](String args) { this->handleSetSSID(args); });
    cli.registerCommand("set_pwd", "Set WiFi Password", 
        [this](String args) { this->handleSetPassword(args); });
    cli.registerCommand("set_ntp", "Set NTP Server", 
        [this](String args) { this->handleSetNTP(args); });
    cli.registerCommand("set_server", "Set Flask Server URL", 
        [this](String args) { this->handleSetServer(args); });
    cli.registerCommand("telemetry", "Enable/Disable Telemetry (on/off)", 
        [this](String args) { this->handleTelemetry(args); });
    cli.registerCommand("web", "Enable/Disable Web Interface (on/off)", 
        [this](String args) { this->handleWebCommand(args); });
    cli.registerCommand("set_mqtt_broker", "Set MQTT Broker", 
        [this](String args) { this->handleSetMqttBroker(args); });
    cli.registerCommand("set_mqtt_port", "Set MQTT Port", 
        [this](String args) { this->handleSetMqttPort(args); });
    cli.registerCommand("set_mqtt_user", "Set MQTT User", 
        [this](String args) { this->handleSetMqttUser(args); });
    cli.registerCommand("set_mqtt_pwd", "Set MQTT Password", 
        [this](String args) { this->handleSetMqttPassword(args); });
    cli.registerCommand("set_ota_pwd", "Set OTA password for espota/ArduinoOTA", 
        [this](String args) { this->handleSetOtaPassword(args); });
    cli.registerCommand("settings", "View current settings", 
        [this](String args) { this->handlePrintSettings(args); });
}

bool SettingsModule::isInitialized() {
    char m[4];
    for (int i = 0; i < 4; i++) {
        m[i] = EEPROM.read(i);
    }
    return (strncmp(m, MAGIC_WORD, 4) == 0);
}

void SettingsModule::load() {
    uint8_t* ptr = (uint8_t*)&settings;
    for (size_t i = 0; i < sizeof(SystemSettings); i++) {
        ptr[i] = EEPROM.read(i);
    }
}

void SettingsModule::save() {
    uint8_t* ptr = (uint8_t*)&settings;
    for (size_t i = 0; i < sizeof(SystemSettings); i++) {
        EEPROM.write(i, ptr[i]);
    }
    EEPROM.commit();
    Logger::info("Settings saved to EEPROM.");
}

void SettingsModule::resetToDefaults() {
    strncpy(settings.magic, MAGIC_WORD, 4);
    strncpy(settings.ssid, "YOUR_WIFI_SSID", sizeof(settings.ssid) - 1);
    settings.ssid[sizeof(settings.ssid) - 1] = '\0';
    
    strncpy(settings.password, "YOUR_WIFI_PASSWORD", sizeof(settings.password) - 1);
    settings.password[sizeof(settings.password) - 1] = '\0';
    
    strncpy(settings.ntpServer, "pool.ntp.org", sizeof(settings.ntpServer) - 1);
    settings.ntpServer[sizeof(settings.ntpServer) - 1] = '\0';
    
    strncpy(settings.serverUrl, "http://192.168.1.100:5000/api/weight", sizeof(settings.serverUrl) - 1);
    settings.serverUrl[sizeof(settings.serverUrl) - 1] = '\0';
    
    settings.telemetryEnabled = false;
    settings.tareOffset = 0;
    settings.sleepIntervalSec = 3600;
    settings.debugMode = false;
    settings.timezoneOffsetSec = 19800; // +5:30 IST default
    settings.fullCylinderWeight = 20000.0f;   // 20kg
    settings.emptyCylinderWeight = 6500.0f;   // 6.5kg
    settings.gasLeakThreshold = 700;
    
    settings.webInterfaceEnabled = false; // Default is off (MQTT is default)
    strncpy(settings.mqttBroker, "homeassistant.local", sizeof(settings.mqttBroker) - 1);
    settings.mqttBroker[sizeof(settings.mqttBroker) - 1] = '\0';
    settings.mqttPort = 1883;
    strncpy(settings.mqttUser, "", sizeof(settings.mqttUser) - 1);
    settings.mqttUser[sizeof(settings.mqttUser) - 1] = '\0';
    strncpy(settings.mqttPassword, "", sizeof(settings.mqttPassword) - 1);
    settings.mqttPassword[sizeof(settings.mqttPassword) - 1] = '\0';
    strncpy(settings.otaPassword, "", sizeof(settings.otaPassword) - 1);
    settings.otaPassword[sizeof(settings.otaPassword) - 1] = '\0';
    
    save();
}

void SettingsModule::handleSetSSID(String args) {
    if (args.length() == 0) {
        Logger::warn("Usage: set_ssid <SSID>");
        return;
    }
    strncpy(settings.ssid, args.c_str(), sizeof(settings.ssid) - 1);
    settings.ssid[sizeof(settings.ssid) - 1] = '\0';
    save();
}

void SettingsModule::handleSetPassword(String args) {
    if (args.length() == 0) {
        Logger::warn("Usage: set_pwd <Password>");
        return;
    }
    strncpy(settings.password, args.c_str(), sizeof(settings.password) - 1);
    settings.password[sizeof(settings.password) - 1] = '\0';
    save();
}

void SettingsModule::handleSetNTP(String args) {
    if (args.length() == 0) {
        Logger::warn("Usage: set_ntp <Server>");
        return;
    }
    strncpy(settings.ntpServer, args.c_str(), sizeof(settings.ntpServer) - 1);
    settings.ntpServer[sizeof(settings.ntpServer) - 1] = '\0';
    save();
}

void SettingsModule::setTelemetryEnabled(bool enabled) {
    settings.telemetryEnabled = enabled;
    save();
}

void SettingsModule::setTareOffset(long offset) {
    settings.tareOffset = offset;
    save();
}

void SettingsModule::setSleepIntervalSec(long sec) {
    settings.sleepIntervalSec = sec;
    save();
}

void SettingsModule::setDebugMode(bool enabled) {
    settings.debugMode = enabled;
    save();
}

void SettingsModule::setTimezoneOffsetSec(long sec) {
    settings.timezoneOffsetSec = sec;
    save();
}

void SettingsModule::setNtpServer(const char* ntp) {
    strncpy(settings.ntpServer, ntp, sizeof(settings.ntpServer) - 1);
    settings.ntpServer[sizeof(settings.ntpServer) - 1] = '\0';
    save();
}

void SettingsModule::setServerUrl(const char* url) {
    strncpy(settings.serverUrl, url, sizeof(settings.serverUrl) - 1);
    settings.serverUrl[sizeof(settings.serverUrl) - 1] = '\0';
    save();
}

void SettingsModule::setFullCylinderWeight(float w) {
    settings.fullCylinderWeight = w;
    save();
}

void SettingsModule::setEmptyCylinderWeight(float w) {
    settings.emptyCylinderWeight = w;
    save();
}

void SettingsModule::setGasLeakThreshold(int ppm) {
    settings.gasLeakThreshold = ppm;
    save();
}

void SettingsModule::setWebInterfaceEnabled(bool enabled) {
    settings.webInterfaceEnabled = enabled;
    save();
}

void SettingsModule::setMqttBroker(const char* broker) {
    strncpy(settings.mqttBroker, broker, sizeof(settings.mqttBroker) - 1);
    settings.mqttBroker[sizeof(settings.mqttBroker) - 1] = '\0';
    save();
}

void SettingsModule::setMqttPort(int port) {
    settings.mqttPort = port;
    save();
}

void SettingsModule::setMqttUser(const char* user) {
    strncpy(settings.mqttUser, user, sizeof(settings.mqttUser) - 1);
    settings.mqttUser[sizeof(settings.mqttUser) - 1] = '\0';
    save();
}

void SettingsModule::setMqttPassword(const char* pwd) {
    strncpy(settings.mqttPassword, pwd, sizeof(settings.mqttPassword) - 1);
    settings.mqttPassword[sizeof(settings.mqttPassword) - 1] = '\0';
    save();
}

void SettingsModule::setOtaPassword(const char* pwd) {
    strncpy(settings.otaPassword, pwd, sizeof(settings.otaPassword) - 1);
    settings.otaPassword[sizeof(settings.otaPassword) - 1] = '\0';
    save();
}

void SettingsModule::handleSetServer(String args) {
    if (args.length() == 0) {
        Logger::warn("Usage: set_server <URL>");
        return;
    }
    strncpy(settings.serverUrl, args.c_str(), sizeof(settings.serverUrl) - 1);
    settings.serverUrl[sizeof(settings.serverUrl) - 1] = '\0';
    save();
}

void SettingsModule::handleTelemetry(String args) {
    if (args == "on") {
        setTelemetryEnabled(true);
        Logger::info("Telemetry Enabled! Will deep sleep and post data automatically.");
    } else if (args == "off") {
        setTelemetryEnabled(false);
        Logger::info("Telemetry Disabled!");
    } else {
        Logger::warn("Usage: telemetry <on/off>");
    }
}

void SettingsModule::handleWebCommand(String args) {
    if (args == "on") {
        setWebInterfaceEnabled(true);
        Logger::info("Web Interface Enabled!");
    } else if (args == "off") {
        setWebInterfaceEnabled(false);
        Logger::info("Web Interface Disabled!");
    } else {
        Logger::warn("Usage: web <on/off>");
    }
}

void SettingsModule::handleSetMqttBroker(String args) {
    if (args.length() == 0) {
        Logger::warn("Usage: set_mqtt_broker <Broker>");
        return;
    }
    setMqttBroker(args.c_str());
    Logger::info("MQTT Broker updated");
}

void SettingsModule::handleSetMqttPort(String args) {
    if (args.length() == 0) {
        Logger::warn("Usage: set_mqtt_port <Port>");
        return;
    }
    setMqttPort(args.toInt());
    Logger::info("MQTT Port updated");
}

void SettingsModule::handleSetMqttUser(String args) {
    if (args.length() == 0) {
        Logger::warn("Usage: set_mqtt_user <User>");
        return;
    }
    setMqttUser(args.c_str());
    Logger::info("MQTT User updated");
}

void SettingsModule::handleSetMqttPassword(String args) {
    if (args.length() == 0) {
        Logger::warn("Usage: set_mqtt_pwd <Password>");
        return;
    }
    setMqttPassword(args.c_str());
    Logger::info("MQTT Password updated");
}

void SettingsModule::handleSetOtaPassword(String args) {
    if (args.length() == 0) {
        Logger::warn("Usage: set_ota_pwd <Password>");
        return;
    }
    setOtaPassword(args.c_str());
    Logger::info("OTA password updated");
}

void SettingsModule::handlePrintSettings(String args) {
    Logger::rawln("=== System Settings ===");
    
    Logger::raw("SSID       : ");
    Logger::rawln(settings.ssid);
    
    Logger::raw("Password   : ");
    // Optional: Hide password for security, but we'll show it or mask it
    Logger::rawln("********"); // masked for security, or show it? Let's show it since it's a dev cli
    //Logger::rawln(settings.password);
    
    Logger::raw("NTP Server : ");
    Logger::rawln(settings.ntpServer);
    
    Logger::raw("Server URL : ");
    Logger::rawln(settings.serverUrl);
    
    Logger::raw("Telemetry  : ");
    Logger::rawln(settings.telemetryEnabled ? "ON" : "OFF");
    
    Logger::raw("Full Cyl   : ");
    Logger::rawln(String(settings.fullCylinderWeight, 0) + "g");
    
    Logger::raw("Empty Cyl  : ");
    Logger::rawln(String(settings.emptyCylinderWeight, 0) + "g");
    
    Logger::raw("Gas Leak   : ");
    Logger::rawln(String(settings.gasLeakThreshold) + " ppm");
    
    Logger::raw("Web UI     : ");
    Logger::rawln(settings.webInterfaceEnabled ? "ON" : "OFF");

    Logger::raw("MQTT Broker: ");
    Logger::rawln(settings.mqttBroker);

    Logger::raw("MQTT Port  : ");
    Logger::rawln(String(settings.mqttPort));

    Logger::raw("MQTT User  : ");
    Logger::rawln(settings.mqttUser);
    
    Logger::rawln("=======================");
}
