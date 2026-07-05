#include "LEDModule.h"
#include "Logger.h"

LEDModule::LEDModule()
    : pin(D4), mode(LEDMode::OFF), lastToggleMs(0), connectedAtMs(0), 
      ledState(false), isStreaming(false) {}

void LEDModule::begin(uint8_t ledPin) {
    pin = ledPin;
    pinMode(pin, OUTPUT);
    setLED(false);  // Start LED off
    Logger::info(F("LED Module initialized on pin D4"));
}

void LEDModule::update() {
    unsigned long now = millis();
    
    // Update WiFi status automatically each cycle
    updateWiFiStatus();
    
    switch (mode) {
        case LEDMode::OFF:
            setLED(false);
            break;
            
        case LEDMode::CONNECTING:
            // Blink slowly while connecting
            if (now - lastToggleMs >= BLINK_CONNECTING_INTERVAL) {
                toggleLED();
                lastToggleMs = now;
            }
            break;
            
        case LEDMode::CONNECTED:
            // Stay on for 5 seconds, then turn off
            if (now - connectedAtMs >= CONNECTED_DURATION) {
                setMode(LEDMode::OFF);
                Logger::info(F("WiFi connected indication complete"));
            } else {
                setLED(true);
            }
            break;
            
        case LEDMode::STREAMING:
            // Blink fast while streaming
            if (now - lastToggleMs >= BLINK_STREAMING_INTERVAL) {
                toggleLED();
                lastToggleMs = now;
            }
            break;
            
        case LEDMode::ERROR:
            // Blink very fast on error
            if (now - lastToggleMs >= BLINK_ERROR_INTERVAL) {
                toggleLED();
                lastToggleMs = now;
            }
            break;
    }
}

void LEDModule::setMode(LEDMode newMode) {
    if (mode != newMode) {
        mode = newMode;
        lastToggleMs = millis();
        
        if (mode == LEDMode::CONNECTED) {
            connectedAtMs = millis();
            setLED(true);
        } else if (mode == LEDMode::OFF) {
            setLED(false);
        }
    }
}

LEDMode LEDModule::getMode() const {
    return mode;
}

void LEDModule::updateWiFiStatus() {
    // Check and update LED based on WiFi status
    wl_status_t wifiStatus = WiFi.status();
    
    if (wifiStatus == WL_CONNECTED) {
        // Only transition to CONNECTED if we're currently connecting
        if (mode == LEDMode::CONNECTING) {
            setMode(LEDMode::CONNECTED);
            Logger::info(F("WiFi connected - LED indicator ON"));
        }
        // If streaming, stay in streaming mode (higher priority)
        // Otherwise stay in current mode
    } else if (wifiStatus == WL_IDLE_STATUS) {
        // WiFi is in the process of connecting
        if (mode != LEDMode::STREAMING && mode != LEDMode::CONNECTED) {
            setMode(LEDMode::CONNECTING);
        }
    } else if (wifiStatus == WL_DISCONNECTED || wifiStatus == WL_NO_SSID_AVAIL || wifiStatus == WL_CONNECT_FAILED) {
        // Disconnected - turn off unless streaming
        if (!isStreaming) {
            setMode(LEDMode::OFF);
        }
    }
}

void LEDModule::setStreaming(bool streaming) {
    isStreaming = streaming;
    
    if (streaming) {
        setMode(LEDMode::STREAMING);
        Logger::info(F("Data streaming - LED blinking fast"));
    } else {
        if (WiFi.status() == WL_CONNECTED) {
            setMode(LEDMode::OFF);  // Return to normal when streaming stops
        }
    }
}

void LEDModule::toggleLED() {
    setLED(!ledState);
}

void LEDModule::setLED(bool on) {
    ledState = on;
    // Note: ESP8266 built-in LED is active LOW (write LOW to turn on)
    digitalWrite(pin, on ? LOW : HIGH);
}
