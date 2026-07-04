#include "SettingsModule.h"
#include "Logger.h"

#define EEPROM_SIZE 512
#define MAGIC_WORD "CFG1"

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
    
    Logger::rawln("=======================");
}
