# Smart LPG Gas Monitor

I built this project to solve a simple household problem: knowing exactly how much gas is left in the LPG cylinder and detecting dangerous gas leaks early. 

Most existing projects use gas pressure to determine the gas level, which requires cutting into the gas line and professional hardware setup. With this product, **you just need to place the cylinder on top of the device.** No extra plumbing, modifications, or professional setup is needed!

Additionally, I implemented a gas leak sensor to detect gas leaks. Because Propane and Butane are heavier than air and sink to the ground, the sensor is placed with the device itself at the base of the cylinder (as a design choice) instead of mounting it high up on a wall.

## Features

- **No-Plumbing Gas Level Monitoring:** Uses a digital scale (HX711 load cell) to precisely measure the weight of the cylinder. Just place the cylinder on top, and it calculates the remaining gas percentage automatically.
- **Built-in Gas Leak Detection:** Uses an MQ-6 sensor to detect LPG gas leaks right at the source where heavy gases pool.
- **Instant Telegram Notifications:** The device connects to the internet to send you an immediate Telegram message if you are running low on gas or if a gas leak is detected.
- **Easy Internet Dashboard:** Connect the device to your WiFi and go directly to its local web dashboard to view live readings, gas gauges, and configure settings directly from your phone or computer.
- **OLED Display:** Shows live system status, gas levels, and startup progress directly on the device.
- **Home Assistant Ready:** For smart home enthusiasts, it can optionally stream data via MQTT for easy integration into Home Assistant or other platforms.

## How It Works

1. **Place & Go:** Set your gas cylinder on the monitor scale. 
2. **Connect WiFi:** Connect the device to your home internet network.
3. **View Dashboard:** Visit the device's IP address on your phone or computer to see the beautifully designed dashboard. 
4. **Get Alerts:** Receive automatic low-gas warnings and emergency leak alerts straight to your phone via Telegram.

## Web Dashboard Login
To access the local web interface, navigate to the device's IP address in your browser. You will be prompted for credentials:
- **Username:** `admin`
- **Password:** *Your OTA Password* (defaults to `admin` if never set)

---
*For Developers & Makers:*
## Build & Upload
This project is built using [PlatformIO](https://platformio.org/).

```bash
pio run -e nodemcuv2 -t upload
```

## CLI Configuration
You can connect via USB (Serial Monitor at 115200 baud) for advanced configuration. Here are some useful commands:
- `tare`: Tare the scale (zero it) when nothing is on it.
- `calibrate <weight>`: Calibrate the scale with a known test weight in grams.
- `set_ssid <SSID>` / `set_pwd <PASSWORD>`: Set WiFi credentials manually.
- `settings`: View all current system configurations.
