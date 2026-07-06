# Smart LPG Gas Monitor

An ESP8266-based Smart LPG (Liquefied Petroleum Gas) Cylinder Monitor. It measures the remaining gas using an HX711 load cell and detects gas leaks using an MQ-6 gas sensor. Data is streamed to Home Assistant via MQTT and can be viewed on a beautifully designed, dark-themed local Web Dashboard.

## Features

- **Gas Cylinder Weight Monitoring:** Uses an HX711 to precisely measure the weight of the cylinder and calculates the remaining gas percentage based on configured empty/full cylinder weights.
- **Gas Leak Detection:** Uses an MQ-6 sensor to detect LPG gas leaks and trigger alerts in ppm.
- **Home Assistant Integration:** Default data output is via MQTT, publishing telemetry every 30 seconds for easy integration into Home Assistant or other smart home platforms.
- **Local Web Dashboard:** A responsive, dark-themed UI hosted directly on the ESP8266 showing circular gauges for gas level, leak status, and live logs. Can be toggled on/off to save resources.
- **LED Status Indicator:** Built-in LED (D4) shows WiFi and data streaming status:
  - **Blinking (slow):** WiFi is attempting to connect
  - **Solid ON (5 sec):** WiFi connection established
  - **Blinking (fast):** Data streaming in progress
- **Advanced Filtering Pipeline:**
  - **Negative Rejection:** Ignores erroneous `< 0` load cell readings.
  - **Median Filter:** A 5-sample moving median window that rejects impulse noise/spikes.
  - **Exponential Moving Average (EMA):** Smooths the median output with an adjustable `alpha` for stable readings.
- **Non-blocking Architecture:** The HX711 and MQTT modules run completely asynchronously, ensuring butter-smooth performance on the Web Interface and Serial CLI without input lag.
- **Terminal CLI:** A modular command-line interface over Serial to easily configure WiFi, MQTT, tare the scale, or debug.
- **NTP Time Sync:** Automatically syncs with NTP servers if an RTC module is not installed or has the wrong time.

## MQTT Topics

The device publishes to the following topics (`lpgmonitor/` prefix):
- `lpgmonitor/weight`: The current weight in kg.
- `lpgmonitor/gas_level`: The remaining gas percentage (0-100%).
- `lpgmonitor/gas_ppm`: The MQ-6 gas sensor reading in ppm.
- `lpgmonitor/status`: "Normal" or "Leak" based on the configured leak threshold.

## CLI Commands

Connect via Serial Monitor (115200 baud by default). Here are some useful commands:

### Hardware Commands
- `tare`: Tare the scale (zero it).
- `calibrate <weight>`: Calibrate the scale with a known weight in grams.
- `stream`: Toggle continuous weight output.
- `gas`: Read the current MQ-6 gas sensor value.
- `gas_threshold <ppm>`: Set the gas leak threshold (default 700).

### Configuration Commands
- `set_ssid <SSID>`: Set the WiFi network name.
- `set_pwd <PASSWORD>`: Set the WiFi password.
- `set_mqtt_broker <URL/IP>`: Set the MQTT broker address (e.g., `homeassistant.local`).
- `set_mqtt_port <PORT>`: Set the MQTT port (default 1883).
- `set_mqtt_user <USER>`: Set the MQTT username.
- `set_mqtt_pwd <PASSWORD>`: Set the MQTT password.
- `telemetry <on/off>`: Enable or disable MQTT telemetry publishing (disabled by default).
- `web <on/off>`: Enable or disable the local web dashboard.
- `settings`: View all current configurations.
- `debug`: Toggle debug logging.

## Build & Upload

This project is built using [PlatformIO](https://platformio.org/).

```bash
pio run -e nodemcuv2 -t upload
```

## Continuous Integration (CI)

This project uses **GitHub Actions** for automated continuous integration. On every push and pull request, the `CI Build` workflow automatically runs a PlatformIO compilation check for the `nodemcuv2` environment to ensure the firmware builds successfully and regressions are caught early.

## Setup & Secrets

- WiFi and MQTT credentials are stored in device EEPROM and can be set via the Serial CLI after first boot. Recommended workflow:
  - Open Serial Monitor at `115200` and use commands like `set_ssid`, `set_pwd`, `set_mqtt_broker`, `set_mqtt_user`, and `set_mqtt_pwd`.
  - Example: `set_ssid MyNetwork` then `set_pwd MyPassword`

- OTA password is intentionally not stored in source. Configure the device OTA password using the CLI:
  - `set_ota_pwd <password>` — this stores the password in EEPROM and is used by the built-in ArduinoOTA/espota server.
  - To upload from your machine with authentication, do not add the password to the repository. Instead run PlatformIO with an upload flag, for example:

```bash
pio run -e nodemcuv2 -t upload --upload-port 192.168.1.102 --upload-flag="--auth=YOUR_OTA_PASSWORD"
```

- For CI or local overrides, create a non-committed `platformio_override.ini` (or pass `--upload-flag`) rather than committing secrets to the repo.

## Notes on Behavior
- WiFi startup is non-blocking: the firmware will attempt to connect in the background so other services (CLI, OTA, web UI) remain available.
- If the HX711 sensor is not detected at boot the device will continue in a degraded mode (no hard halt) so you can still access OTA and CLI for troubleshooting.

## LED Status Indicator (D4)

The built-in LED on D4 provides visual feedback on system state:

- **OFF:** No WiFi or error state
- **CONNECTING (slow blink 600ms):** WiFi connection attempt in progress
- **CONNECTED (solid on 5s):** WiFi successfully connected; LED stays on for 5 seconds then turns off
- **STREAMING (fast blink 200ms):** Data streaming via scale or sensors (highest priority)
- **ERROR (very fast blink 100ms):** Error condition (reserved for future use)

LED modes update automatically based on WiFi status and data streaming state. The LED provides real-time status without needing serial console access.

## Memory Optimizations

This firmware implements several memory management best practices for constrained ESP8266 environments:

- **Flash String Storage (`F()` macro):** All constant string literals in logging and CLI are wrapped with the `F()` macro to store them in flash memory instead of RAM. This significantly reduces heap fragmentation and preserves RAM for runtime data and buffers.
- **Non-blocking WiFi:** The WiFi connection attempt in `setup()` is non-blocking with asynchronous status checking in `loop()`, preventing long delays that would block other essential tasks.
- **MQTT Backoff:** Implements exponential backoff with jitter for failed MQTT reconnection attempts, reducing network thrashing and improving graceful degradation.
- **Efficient Filtering:** The scale driver uses a compact median buffer (configurable window size) and EMA smoothing without unnecessary dynamic allocations.
- **Sensor Degradation:** If hardware (HX711, RTC) fails to initialize, the system continues in degraded mode instead of halting, allowing OTA updates and CLI access for troubleshooting.
- **EEPROM Integrity:** Uses `static_assert` to prevent silent corruption if settings exceed EEPROM size, avoiding memory boundary overwrites.
- **PROGMEM HTML:** Web Interface dashboard HTML is stored entirely in flash memory (`PROGMEM`) instead of RAM, saving ~8KB of precious heap space on the ESP8266.
- **Circular Log Buffer:** Live logs are managed in a static circular buffer to prevent heap fragmentation and OOM crashes caused by `String` reallocation over long runtimes.
- **Fully Non-blocking Network Stack:** MQTT TCP connect timeouts and NTP time synchronization loops are non-blocking or strictly bounded, preventing the firmware from freezing when services are unreachable.
