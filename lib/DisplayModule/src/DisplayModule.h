#ifndef DISPLAY_MODULE_H
#define DISPLAY_MODULE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ScaleDriver.h"
#include "GasSensorModule.h"
#include "SettingsModule.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

class DisplayModule {
public:
    DisplayModule(ScaleDriver& scale, GasSensorModule& gas, SettingsModule& settings);
    
    void begin();
    void update();

private:
    ScaleDriver& scaleDriver;
    GasSensorModule& gasSensor;
    SettingsModule& settingsModule;
    Adafruit_SSD1306 display;
    
    unsigned long lastUpdateMs;
};

#endif
