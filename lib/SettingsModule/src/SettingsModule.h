#ifndef SETTINGS_MODULE_H
#define SETTINGS_MODULE_H

#include <Arduino.h>
#include <EEPROM.h>
#include "TerminalCLI.h"

struct SystemSettings {
    char magic[4]; // To check if EEPROM is initialized
    char ssid[33];
    char password[65];
    char ntpServer[65];
    char serverUrl[129];
    bool telemetryEnabled;
    long tareOffset;
    long sleepIntervalSec;
    bool debugMode;
    long timezoneOffsetSec;
    float fullCylinderWeight;    // Weight of a full cylinder in grams (default 20000g = 20kg)
    float emptyCylinderWeight;   // Weight of empty cylinder shell in grams (default 6500g = 6.5kg)
    int gasLeakThreshold;        // MQ-6 ppm threshold for leak alert (default 700)
    bool webInterfaceEnabled;
    char mqttBroker[65];
    int mqttPort;
    char mqttUser[33];
    char mqttPassword[65];
    char otaPassword[65];
    float calibrationFactor;     // Scale calibration factor
};

class SettingsModule {
public:
    SettingsModule();
    void begin(TerminalCLI& cli);
    
    const char* getSSID() const { return settings.ssid; }
    const char* getPassword() const { return settings.password; }
    const char* getNtpServer() const { return settings.ntpServer; }
    const char* getServerUrl() const { return settings.serverUrl; }
    bool isTelemetryEnabled() const { return settings.telemetryEnabled; }
    long getTareOffset() const { return settings.tareOffset; }
    long getSleepIntervalSec() const { return settings.sleepIntervalSec; }
    bool isDebugMode() const { return settings.debugMode; }
    long getTimezoneOffsetSec() const { return settings.timezoneOffsetSec; }
    float getFullCylinderWeight() const { return settings.fullCylinderWeight; }
    float getEmptyCylinderWeight() const { return settings.emptyCylinderWeight; }
    int getGasLeakThreshold() const { return settings.gasLeakThreshold; }
    bool isWebInterfaceEnabled() const { return settings.webInterfaceEnabled; }
    const char* getMqttBroker() const { return settings.mqttBroker; }
    int getMqttPort() const { return settings.mqttPort; }
    const char* getMqttUser() const { return settings.mqttUser; }
    const char* getMqttPassword() const { return settings.mqttPassword; }
    const char* getOtaPassword() const { return settings.otaPassword; }
    float getCalibrationFactor() const { return settings.calibrationFactor; }
    
    void setTelemetryEnabled(bool enabled);
    void setTareOffset(long offset);
    void setSleepIntervalSec(long sec);
    void setDebugMode(bool enabled);
    void setTimezoneOffsetSec(long sec);
    void setNtpServer(const char* ntp);
    void setServerUrl(const char* url);
    void setSSID(const char* ssid);
    void setPassword(const char* pwd);
    void setFullCylinderWeight(float w);
    void setEmptyCylinderWeight(float w);
    void setGasLeakThreshold(int ppm);
    void setWebInterfaceEnabled(bool enabled);
    void setMqttBroker(const char* broker);
    void setMqttPort(int port);
    void setMqttUser(const char* user);
    void setMqttPassword(const char* pwd);
    void setOtaPassword(const char* pwd);
    void setCalibrationFactor(float factor);

private:
    SystemSettings settings;
    bool isInitialized();
    void load();
    void save();
    void resetToDefaults();

    // CLI Handlers
    void handleSetSSID(String args);
    void handleSetPassword(String args);
    void handleSetNTP(String args);
    void handleSetServer(String args);
    void handleTelemetry(String args);
    void handlePrintSettings(String args);
    void handleWebCommand(String args);
    void handleSetMqttBroker(String args);
    void handleSetMqttPort(String args);
    void handleSetMqttUser(String args);
    void handleSetMqttPassword(String args);
    void handleSetOtaPassword(String args);
};

#endif
