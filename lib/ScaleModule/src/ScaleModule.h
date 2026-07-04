#ifndef SCALE_MODULE_H
#define SCALE_MODULE_H

#include <Arduino.h>
#include "ScaleDriver.h"
#include "TerminalCLI.h"
#include "SettingsModule.h"

class ScaleModule {
public:
    ScaleModule(ScaleDriver& driver);
    
    // Registers scale commands to the provided CLI
    void begin(TerminalCLI& cli, SettingsModule& settings);
    
    // To be called in loop() - updates the driver and handles streaming output
    void update();

private:
    ScaleDriver& scaleDriver;
    SettingsModule* settings;
    
    // Command handlers
    void handleTare(String args);
    void handleCalibrate(String args);
};

#endif
