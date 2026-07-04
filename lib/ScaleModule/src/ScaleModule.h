#ifndef SCALE_MODULE_H
#define SCALE_MODULE_H

#include <Arduino.h>
#include "ScaleDriver.h"
#include "TerminalCLI.h"

class ScaleModule {
public:
    ScaleModule(ScaleDriver& driver);
    
    // Registers scale commands to the provided CLI
    void begin(TerminalCLI& cli);
    
    // To be called in loop() - updates the driver and handles streaming output
    void update();

private:
    ScaleDriver& scaleDriver;
    bool isStreamingData;
    
    // Command handlers
    void handleTare(String args);
    void handleCalibrate(String args);
    void handleStream(String args);
};

#endif
