# Home Assistant Setup Guide

To set up Home Assistant to work with your Smart LPG Monitor, you need to configure the device to talk to your broker, and then tell Home Assistant how to read the data using MQTT sensors.

Here is a step-by-step guide:

### Step 1: Configure the ESP8266 (The Device)
Connect to your device using the Serial Monitor (115200 baud) and run the following commands to configure your MQTT credentials and enable telemetry:

```text
set_mqtt_broker <your_home_assistant_ip>
set_mqtt_port 1883
set_mqtt_user <your_mqtt_username>
set_mqtt_pwd <your_mqtt_password>
telemetry on
```
*(Note: If you don't use a username/password for MQTT, you can skip those two commands).*

### Step 2: Ensure MQTT is running on Home Assistant
Make sure you have an MQTT broker installed in Home Assistant. The most common way is to go to **Settings > Add-ons > Add-on Store** and install the **Mosquitto broker**. Once installed and started, go to **Settings > Devices & Services** and add the **MQTT integration**.

### Step 3: Add Sensors to Home Assistant (`configuration.yaml`)
You will need to manually configure the sensors since the device doesn't use MQTT Auto-Discovery. Open your Home Assistant `configuration.yaml` file (you can use the File Editor or Studio Code Server add-on) and add the following:

```yaml
mqtt:
  sensor:
    - name: "LPG Cylinder Weight"
      state_topic: "lpgmonitor/weight"
      unit_of_measurement: "kg"
      icon: "mdi:weight-kilogram"
      device_class: "weight"
      state_class: "measurement"

    - name: "LPG Gas Level"
      state_topic: "lpgmonitor/gas_level"
      unit_of_measurement: "%"
      icon: "mdi:gas-cylinder"
      device_class: "battery"
      state_class: "measurement"

    - name: "LPG Gas PPM"
      state_topic: "lpgmonitor/gas_ppm"
      unit_of_measurement: "ppm"
      icon: "mdi:molecule-co2"
      state_class: "measurement"

    - name: "LPG Leak Status"
      state_topic: "lpgmonitor/status"
      icon: "mdi:shield-alert"
```

### Step 4: Restart Home Assistant
Once you've saved your `configuration.yaml`, go to **Developer Tools > Check Configuration**. If it says the configuration is valid, click **Restart**.

### Step 5: Add to your Home Assistant Dashboard
After Home Assistant reboots, you will have four new entities available:
* `sensor.lpg_cylinder_weight`
* `sensor.lpg_gas_level`
* `sensor.lpg_gas_ppm`
* `sensor.lpg_leak_status`

You can go to your Overview dashboard, click **Edit Dashboard**, and add an **Entities Card** or a **Gauge Card** to beautifully display your gas levels and leak status!
