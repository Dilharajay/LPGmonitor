#include "ScaleModule.h"
#include "Logger.h"

ScaleModule::ScaleModule(ScaleDriver& driver) : 
    scaleDriver(driver), settings(nullptr) {}

void ScaleModule::begin(TerminalCLI& cli, SettingsModule& s) {
    settings = &s;
    // Register commands with lambda bindings to this instance
    cli.registerCommand("t", "Tare the scale (zero it)", 
                        [this](String args) { this->handleTare(args); });
    cli.registerCommand("tare", "Tare the scale (zero it)", 
                        [this](String args) { this->handleTare(args); });

    cli.registerCommand("c", "Calibrate with a known weight (e.g., c 148)", 
                        [this](String args) { this->handleCalibrate(args); });
    cli.registerCommand("calibrate", "Calibrate with a known weight", 
                        [this](String args) { this->handleCalibrate(args); });
}

void ScaleModule::handleTare(String args) {
    Logger::info("Taring... please wait.");
    long newOffset = scaleDriver.performTare();
    if (newOffset != 0 && settings) {
        settings->setTareOffset(newOffset);
        Logger::info("New tare offset saved to EEPROM.");
    }
}

void ScaleModule::handleCalibrate(String args) {
    float knownWeight = args.toFloat();
    scaleDriver.performCalibration(knownWeight);
}

void ScaleModule::update() {
    // 1. Update hardware
    scaleDriver.update();
}
