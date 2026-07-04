#ifndef SCALE_DRIVER_H
#define SCALE_DRIVER_H

#include <Arduino.h>
#include <HX711.h>
#include "Config.h"

class ScaleDriver {
public:
    ScaleDriver();
    
    // Initialize the HX711 and perform initial tare
    void begin(uint8_t doutPin, uint8_t sckPin, float calFactor);
    
    // Commands
    void performTare();
    void performCalibration(float knownWeight);
    
    // Read the next sample and run it through the filter pipeline
    void update();
    
    // Getters
    float getFilteredWeight();
    float getCalibrationFactor();

private:
    HX711 scale;
    float calibrationFactor;

    // ── Median filter ──────────────────────────────────────────────
    //  Collects MEDIAN_WINDOW raw samples, sorts a copy, picks the
    //  middle value.  This kills impulse noise / spikes.
    float medianBuffer[Config::MEDIAN_WINDOW];
    int   medianIndex;
    int   medianCount;          // how many valid samples so far

    float computeMedian();      // returns median of the current buffer

    // ── Exponential Moving Average (EMA) ───────────────────────────
    //  Smooths the median-filtered output over time.
    //  alpha  = 0.0–1.0  (higher = less smoothing, faster response)
    float emaValue;
    bool  emaInitialized;

    // ── Shared helpers ─────────────────────────────────────────────
    void resetFilters();
};

#endif
