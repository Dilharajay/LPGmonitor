# Digital Scale Firmware

Modular ESP8266-based digital scale firmware, featuring a flexible CLI system and advanced signal filtering.

## Features
- **Generic Terminal CLI:** A modular, extensible command-line interface that echoes input, handles backspaces, and auto-generates help menus.
- **Scale Module:** Encapsulates the HX711 scale driver, providing commands for tare, calibration, and streaming.
- **Advanced Filtering Pipeline:**
  - **Negative Rejection:** Ignores erroneous `< 0` readings.
  - **Median Filter:** A 5-sample moving median window that rejects impulse noise/spikes.
  - **Exponential Moving Average (EMA):** Smooths the median output with an adjustable `alpha` (default 0.15) for stable readings.
- **Centralized Configuration:** Tune hardware pins, serial baud rate, and filter settings in a single `include/Config.h` file.
- **Smart Logger:** Tagged logging levels (`DEBUG`, `INFO`, `WARN`, `ERROR`) with millisecond uptimes.

## CLI Commands
Connect via Serial Monitor (115200 baud by default):
- `t` or `tare`: Tare the scale (zero it).
- `c <weight>` or `calibrate <weight>`: Calibrate the scale with a known weight in grams.
- `s` or `stream`: Toggle continuous weight output.
- `d` or `debug`: Toggle debug logging.
- `help` or `?`: Display the available commands.

## Architecture
The system is built to easily support new hardware modules (e.g., displays, motors).
Just define your module, register its commands using `cli.registerCommand()`, and update it in `main.cpp`!

## Build & Upload
This project is built using PlatformIO.

```bash
pio run -e nodemcuv2 -t upload
```
