#include "TelemetryModule.h"
#include "Logger.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

TelemetryModule::TelemetryModule(ScaleDriver& scaleDriver, TimeModule& timeMod)
    : settings(nullptr), scale(scaleDriver), timeModule(timeMod) {}

void TelemetryModule::begin(TerminalCLI& cli, SettingsModule& s) {
    settings = &s;
    
    if (!settings->isTelemetryEnabled()) {
        return; // Telemetry is off, continue to normal operation
    }

    Logger::info("Telemetry Mode Active. Waiting 5 seconds before deep sleep loop...");
    Logger::info("Type 'telemetry off' in this window to cancel!");

    unsigned long start = millis();
    while (millis() - start < 5000) {
        cli.handle();
        delay(10);
        if (!settings->isTelemetryEnabled()) {
            Logger::info("Telemetry aborted by user.");
            return;
        }
    }

    // Connect to WiFi if not connected
    if (WiFi.status() != WL_CONNECTED) {
        Logger::info("Telemetry: Connecting to WiFi...");
        WiFi.begin(settings->getSSID(), settings->getPassword());
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Logger::raw(".");
            attempts++;
        }
        Logger::rawln();
    }

    if (WiFi.status() != WL_CONNECTED) {
        Logger::error("Telemetry: WiFi connection failed! Going to sleep anyway...");
    } else {
        // We are connected. Wait for scale to settle just in case
        for (int i = 0; i < 20; i++) {
            scale.update();
            delay(100);
        }

        // Prepare JSON payload
        float weight = scale.getFilteredWeight();
        String timeStr = timeModule.getTimeString();
        
        String payload = "{\"timestamp\":\"" + timeStr + "\",\"weight_g\":" + String(weight, 2) + "}";
        
        Logger::info("Sending Payload: ");
        Logger::info(payload.c_str());

        // Send HTTP POST
        WiFiClient client;
        HTTPClient http;
        http.begin(client, settings->getServerUrl());
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(payload);
        if (httpCode > 0) {
            Logger::info("Telemetry payload sent successfully. HTTP Code: ");
            Logger::info(String(httpCode).c_str());

            if (httpCode == 200) {
                String response = http.getString();
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, response);

                if (!error && doc["update_config"] == true) {
                    Logger::info("Received new configuration from server!");
                    
                    if (!doc["config"]["telemetry"].isNull()) {
                        bool enable = (doc["config"]["telemetry"] == "on");
                        settings->setTelemetryEnabled(enable);
                        Logger::info(enable ? "Telemetry updated: ON" : "Telemetry updated: OFF");
                    }
                    if (!doc["config"]["ntp_server"].isNull()) {
                        const char* ntp = doc["config"]["ntp_server"];
                        settings->setNtpServer(ntp);
                        Logger::info("NTP Server updated.");
                    }
                }
            }
        } else {
            Logger::error("Failed to send telemetry payload. Error: ");
            Logger::error(http.errorToString(httpCode).c_str());
        }
        http.end();
        
        WiFi.disconnect(true);
    }

    // Sleep for 1 hour (3600 seconds = 3600 * 1000 * 1000 microseconds)
    // IMPORTANT: GPIO16 (D0) must be connected to RST for the ESP8266 to wake up!
    Logger::info("Going to Deep Sleep for 1 hour... (Connect D0 to RST!)");
    ESP.deepSleep(3600e6);
}
