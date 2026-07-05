#include "GasSensorModule.h"
#include "Logger.h"
#include "Config.h"

GasSensorModule::GasSensorModule() :
    rawValue(0), ppmValue(0), leakThreshold(700), lastReadMs(0) {}

void GasSensorModule::begin(TerminalCLI& cli) {
    pinMode(A0, INPUT);
    Logger::info(F("GasSensorModule: MQ-6 sensor initialized on A0"));

    cli.registerCommand("gas", "Show current gas sensor PPM and raw value",
                        [this](String args) {
        Logger::raw(F("Gas PPM: "));
        Logger::raw(ppmValue);
        Logger::raw(F("  Raw: "));
        Logger::rawln(rawValue);
        if (isLeakDetected()) {
            Logger::warn(F("!! GAS LEAK DETECTED !!"));
        }
    });

    cli.registerCommand("gas_threshold", "Set gas leak threshold PPM (e.g., gas_threshold 800)",
                        [this](String args) {
        if (args.length() == 0) {
            Logger::raw(F("Current threshold: "));
            Logger::raw(leakThreshold);
            Logger::rawln(F(" ppm"));
            return;
        }
        int val = args.toInt();
        if (val > 0) {
            leakThreshold = val;
            Logger::raw(F("Leak threshold set to "));
            Logger::raw(leakThreshold);
            Logger::rawln(F(" ppm"));
        } else {
            Logger::warn(F("Invalid threshold value"));
        }
    });
}

void GasSensorModule::update() {
    unsigned long now = millis();
    if (now - lastReadMs < 500) return;
    lastReadMs = now;

    rawValue = analogRead(A0);
    ppmValue = convertToPPM(rawValue);
}

int GasSensorModule::getRawValue() {
    return rawValue;
}

int GasSensorModule::getPPM() {
    return ppmValue;
}

bool GasSensorModule::isLeakDetected() {
    return ppmValue >= leakThreshold;
}

void GasSensorModule::setLeakThreshold(int ppm) {
    leakThreshold = ppm;
}

int GasSensorModule::getLeakThreshold() {
    return leakThreshold;
}

int GasSensorModule::convertToPPM(int rawVal) {
    // Simple linear approximation for MQ-6: maps 0-1023 ADC to 0-10000 ppm
    return (int)((rawVal / 1023.0) * 10000);
}
