#include "DisplayModule.h"
#include "Logger.h"
#include <ESP8266WiFi.h>

DisplayModule::DisplayModule(ScaleModule& scale, GasSensorModule& gas)
    : scaleModule(scale), gasSensor(gas), display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), lastUpdateMs(0) {
}

void DisplayModule::begin() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
        Logger::error(F("SSD1306 allocation failed"));
        return;
    }
    
    Logger::info(F("OLED Display Initialized"));
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("Smart LPG Monitor"));
    display.println(F("Starting up..."));
    display.display();
}

void DisplayModule::update() {
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
    
    // 2. Gas Level (Big Text)
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.print(F("Gas: "));
    display.print((int)scaleModule.getGasPercentage());
    display.print(F("%"));
    
    // 3. Weight Details
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print(F("Weight: "));
    display.print(scaleModule.getWeightKg(), 2);
    display.print(F(" kg"));
    
    // 4. Gas Sensor PPM
    display.setCursor(0, 45);
    display.print(F("Sensor: "));
    display.print(gasSensor.getPPM());
    display.print(F(" ppm"));
    
    display.display();
}
