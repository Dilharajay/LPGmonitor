#ifndef GAS_SENSOR_MODULE_H
#define GAS_SENSOR_MODULE_H

#include <Arduino.h>
#include "TerminalCLI.h"

class GasSensorModule {
public:
    GasSensorModule();
    void begin(TerminalCLI& cli);
    void update();
    
    int getRawValue();       // Raw ADC 0-1023
    int getPPM();            // Approximate PPM value
    bool isLeakDetected();   // True if above threshold
    bool isWarmingUp() const; // True if sensor is within 10m warmup period
    
    void setLeakThreshold(int ppm);
    int getLeakThreshold();

private:
    int rawValue;
    int ppmValue;
    int leakThreshold;       // Default 700 ppm
    unsigned long lastReadMs;
    
    int convertToPPM(int rawVal);
};

#endif
