#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace Config {

    // Project info
    constexpr const char* PROJECT_NAME = "Smart LPG Monitor";
    constexpr const char* PROJECT_VERSION = "1.0.0";

    // OTA Update Settings
    constexpr const char* OTA_HOSTNAME = "SmartLPGMonitor";
    constexpr const char* OTA_PASSWORD = "lpg123";
    // ── Hardware Pins ──
    constexpr uint8_t HX711_DOUT_PIN = D2; // GPIO4 (D2 on NodeMCU)
    constexpr uint8_t HX711_SCK_PIN  = D1; // GPIO5 (D1 on NodeMCU)
    
    // I2C Pins for DS1307 RTC
    constexpr uint8_t I2C_SDA_PIN = D6; 
    constexpr uint8_t I2C_SCL_PIN = D7;

    // ── Serial Settings ──
    constexpr uint32_t SERIAL_BAUD_RATE = 115200;

    // ── Scale Settings ──
    constexpr float DEFAULT_CALIBRATION_FACTOR = 103.4f;

    // ── Filter Settings ──
    // Median window kills impulse noise/spikes. Larger = more spike resistance, slower response.
    // NOTE: If you change this, make sure to update ScaleDriver accordingly.
    constexpr int MEDIAN_WINDOW = 5;
    
    // EMA smooths the median-filtered output over time. 
    // Range: 0.0 to 1.0 (higher = less smoothing, faster response)
    constexpr float EMA_ALPHA = 0.15f;

    // ── WiFi & NTP Settings ──
    constexpr long GMT_OFFSET_SEC = 0;           // Change for your timezone (e.g. 3600 for GMT+1)
    constexpr int  DAYLIGHT_OFFSET_SEC = 0;      // Daylight saving time offset in seconds

    // ── CLI Settings ──
    // Maximum number of commands that can be registered in the CLI
    constexpr int MAX_CLI_COMMANDS = 32;
}

#endif // CONFIG_H
