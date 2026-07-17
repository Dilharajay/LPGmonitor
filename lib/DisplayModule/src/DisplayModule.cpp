#include "DisplayModule.h"
#include "Logger.h"
#include <ESP8266WiFi.h>

extern uint32_t currentLoopTimeMs;

DisplayModule::DisplayModule(ScaleDriver& scale, GasSensorModule& gas, SettingsModule& settings)
    : scaleDriver(scale), gasSensor(gas), settingsModule(settings), display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), lastUpdateMs(0), inOtaMode(false) {
}

void DisplayModule::begin() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
        Logger::error(F("SSD1306 allocation failed"));
        return;
    }
    
    Logger::info(F("OLED Display Initialized"));
    showBootScreen("BIOS INIT", 0);
}

void DisplayModule::showBootScreen(const String& task, int progress) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Header
    display.setCursor(0, 0);
    display.println(F("== LPG BIOS v1.0 =="));
    
    // Task
    display.setCursor(0, 20);
    display.print(F("> "));
    display.println(task);
    
    // Progress Bar
    if (progress >= 0 && progress <= 100) {
        display.drawRect(0, 40, 128, 10, SSD1306_WHITE);
        display.fillRect(2, 42, (124 * progress) / 100, 6, SSD1306_WHITE);
    }
    
    display.display();
}

void DisplayModule::showOtaScreen(int progress) {
    inOtaMode = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(0, 0);
    display.println(F("SYSTEM UPDATE"));
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    display.setCursor(0, 20);
    display.print(F("Flashing OTA... "));
    display.print(progress);
    display.println(F("%"));
    
    display.drawRect(0, 40, 128, 10, SSD1306_WHITE);
    display.fillRect(2, 42, (124 * progress) / 100, 6, SSD1306_WHITE);
    
    display.display();
}

void DisplayModule::update() {
    if (inOtaMode) return; // Halt UI updates during OTA
    
    unsigned long now = millis();
    // Update screen every 500ms
    if (now - lastUpdateMs < 500) return;
    lastUpdateMs = now;
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // 1. Top bar: WiFi and Status
    display.setTextSize(1);
    display.setCursor(0, 0);
    if (WiFi.status() == WL_CONNECTED) {
        display.print(F("WiFi:OK "));
    } else {
        display.print(F("WiFi:X  "));
    }
    
    display.setCursor(80, 0);
    if (gasSensor.isLeakDetected()) {
        display.print(F("!LEAK!"));
    } else {
        display.print(F("SAFE"));
    }
    
    // Draw a line separator
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Calculate Gas %
    float weight = scaleDriver.getFilteredWeight();
    float emptyWeight = settingsModule.getEmptyCylinderWeight();
    float fullWeight = settingsModule.getFullCylinderWeight();
    float gasCapacity = fullWeight - emptyWeight;
    
    float gasPercent = 0.0f;
    if (gasCapacity > 0.0f) {
        float gasRemaining = weight - emptyWeight;
        gasPercent = (gasRemaining / gasCapacity) * 100.0f;
        if (gasPercent < 0.0f) gasPercent = 0.0f;
        if (gasPercent > 100.0f) gasPercent = 100.0f;
    }
    
    // 2. Gas Level (Big Text)
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.print(F("Gas: "));
    display.print((int)gasPercent);
    display.print(F("%"));
    
    // 3. Weight Details
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print(F("Weight: "));
    display.print(weight, 2);
    display.print(F(" kg"));
    
    // 4. Sensor & System Stats
    display.setCursor(0, 45);
    display.print(F("Sensor: "));
    display.print(gasSensor.getPPM());
    display.print(F(" ppm"));
    
    display.setCursor(0, 55);
    uint32_t freeRam = ESP.getFreeHeap() / 1024;
    display.print(F("RAM:"));
    display.print(freeRam);
    display.print(F("KB "));
    
    display.print(F(" CPU:"));
    display.print(currentLoopTimeMs); // We'll show loop time as a proxy for CPU load
    display.print(F("ms"));
    
    display.display();
}
