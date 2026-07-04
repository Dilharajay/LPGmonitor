#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace Config {
    // ── Hardware Pins ──
    constexpr uint8_t HX711_DOUT_PIN = D2; // GPIO4 (D2 on NodeMCU)
    constexpr uint8_t HX711_SCK_PIN  = D1; // GPIO5 (D1 on NodeMCU)

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

    // ── CLI Settings ──
    // Maximum number of commands that can be registered in the CLI
    constexpr int MAX_CLI_COMMANDS = 32;
}

#endif // CONFIG_H
