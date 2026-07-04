#include "ScaleModule.h"
#include "Logger.h"

ScaleModule::ScaleModule(ScaleDriver& driver) : 
    scaleDriver(driver), isStreamingData(false) {}

void ScaleModule::begin(TerminalCLI& cli) {
    // Register commands with lambda bindings to this instance
    cli.registerCommand("t", "Tare the scale (zero it)", 
                        [this](String args) { this->handleTare(args); });
    cli.registerCommand("tare", "Tare the scale (zero it)", 
                        [this](String args) { this->handleTare(args); });

    cli.registerCommand("c", "Calibrate with a known weight (e.g., c 148)", 
                        [this](String args) { this->handleCalibrate(args); });
    cli.registerCommand("calibrate", "Calibrate with a known weight", 
                        [this](String args) { this->handleCalibrate(args); });

    cli.registerCommand("s", "Toggle continuous weight streaming", 
                        [this](String args) { this->handleStream(args); });
    cli.registerCommand("stream", "Toggle continuous weight streaming", 
                        [this](String args) { this->handleStream(args); });
}

void ScaleModule::handleTare(String args) {
    Logger::info("Taring... please wait.");
    scaleDriver.performTare();
}

void ScaleModule::handleCalibrate(String args) {
    float knownWeight = args.toFloat();
    scaleDriver.performCalibration(knownWeight);
}

void ScaleModule::handleStream(String args) {
    isStreamingData = !isStreamingData;
    Logger::info(isStreamingData ? "Data streaming: ON" : "Data streaming: OFF");
}

void ScaleModule::update() {
    // 1. Update hardware
    scaleDriver.update();
    
    // 2. Output stream if enabled
    if (isStreamingData) {
        float weight = scaleDriver.getFilteredWeight();
        
        Logger::raw("Weight: ");
        Logger::raw(weight, 1);
        Logger::raw(" g    ");
        Logger::raw(weight / 1000.0, 3);
        Logger::rawln(" kg");
    }
}
