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

bool GasSensorModule::isWarmingUp() const {
    return millis() < 600000; // 10 minutes warmup
}

bool GasSensorModule::isLeakDetected() {
    if (isWarmingUp()) return false;
    return ppmValue >= leakThreshold;
}

void GasSensorModule::setLeakThreshold(int ppm) {
    leakThreshold = ppm;
}

int GasSensorModule::getLeakThreshold() {
    return leakThreshold;
}

int GasSensorModule::convertToPPM(int rawVal) {
    // The sensor outputs 0-5V.
    // The 2.2k + 2.2k voltage divider halves this to 0-2.5V at the A0 pin.
    // NodeMCU ADC (0-1023) maps to 0-3.3V.
    // So 2.5V on the A0 pin corresponds to an ADC value of (2.5 / 3.3) * 1023 = 775.
    // Simple linear approximation: maps 0-775 ADC to 0-10000 ppm
    int ppm = (int)((rawVal / 775.0) * 10000);
    return (ppm > 10000) ? 10000 : ppm; // Cap at 10000 ppm
}
