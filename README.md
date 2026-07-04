# Smart LPG Gas Monitor

An ESP8266-based Smart LPG (Liquefied Petroleum Gas) Cylinder Monitor. It measures the remaining gas using an HX711 load cell and detects gas leaks using an MQ-6 gas sensor. Data is streamed to Home Assistant via MQTT and can be viewed on a beautifully designed, dark-themed local Web Dashboard.

## Features

- **Gas Cylinder Weight Monitoring:** Uses an HX711 to precisely measure the weight of the cylinder and calculates the remaining gas percentage based on configured empty/full cylinder weights.
- **Gas Leak Detection:** Uses an MQ-6 sensor to detect LPG gas leaks and trigger alerts in ppm.
- **Home Assistant Integration:** Default data output is via MQTT, publishing telemetry every 30 seconds for easy integration into Home Assistant or other smart home platforms.
- **Local Web Dashboard:** A responsive, dark-themed UI hosted directly on the ESP8266 showing circular gauges for gas level, leak status, and live logs. Can be toggled on/off to save resources.
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
