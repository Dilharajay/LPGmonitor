#ifndef LED_MODULE_H
#define LED_MODULE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

enum class LEDMode {
    OFF,             // LED off
    CONNECTING,      // Blink slow (WiFi connecting)
    CONNECTED,       // Solid on for 5 seconds (WiFi connected)
    STREAMING,       // Blink fast (data streaming)
    ERROR            // Blink very fast (error state)
};

class LEDModule {
public:
    LEDModule();
    
    // Initialize LED on specified pin (default D4 for NodeMCU)
    void begin(uint8_t ledPin = D4);
    
    // Update LED state - call this every loop iteration
    void update();
    
    // Set LED mode
    void setMode(LEDMode mode);
    LEDMode getMode() const;
    
    // WiFi connection helpers
    void updateWiFiStatus();
    
    // Data streaming helper
    void setStreaming(bool streaming);

private:
    uint8_t pin;
    LEDMode mode;
    unsigned long lastToggleMs;
    unsigned long connectedAtMs;
    bool ledState;  // true = on, false = off
    bool isStreaming;
    
    // Blink timing (milliseconds)
    static constexpr unsigned long BLINK_CONNECTING_INTERVAL = 600;    // Slow blink
    static constexpr unsigned long BLINK_STREAMING_INTERVAL = 200;     // Fast blink
    static constexpr unsigned long BLINK_ERROR_INTERVAL = 100;         // Very fast blink
    static constexpr unsigned long CONNECTED_DURATION = 5000;          // 5 seconds solid
    
    void toggleLED();
    void setLED(bool on);
};

#endif // LED_MODULE_H
