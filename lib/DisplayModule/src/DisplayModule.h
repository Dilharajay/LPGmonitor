#ifndef DISPLAY_MODULE_H
#define DISPLAY_MODULE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ScaleModule.h"
#include "GasSensorModule.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

class DisplayModule {
public:
    DisplayModule(ScaleModule& scale, GasSensorModule& gas);
    
    void begin();
    void update();

private:
    ScaleModule& scaleModule;
    GasSensorModule& gasSensor;
    Adafruit_SSD1306 display;
    
    unsigned long lastUpdateMs;
};

#endif
