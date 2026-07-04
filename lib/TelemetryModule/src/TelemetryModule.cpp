#include "TelemetryModule.h"
#include "Logger.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

TelemetryModule::TelemetryModule(ScaleDriver& scaleDriver, TimeModule& timeMod)
    : settings(nullptr), scale(scaleDriver), timeModule(timeMod), lastPostTime(0) {}

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

        postData();
    }

    if (!settings->isDebugMode()) {
        WiFi.disconnect(true);
        long sleepInterval = settings->getSleepIntervalSec();
        if (sleepInterval <= 0) sleepInterval = 3600;
        Logger::info("Going to Deep Sleep for configured interval... (Connect D0 to RST!)");
        ESP.deepSleep(sleepInterval * 1000000ULL);
    } else {
        Logger::info("Debug Mode ON. Staying awake to stream telemetry.");
        lastPostTime = millis();
    }
}

void TelemetryModule::update() {
    if (!settings || !settings->isTelemetryEnabled() || !settings->isDebugMode()) return;
    
    // In debug mode, stream every 1 second
    if (millis() - lastPostTime > 1000) {
        lastPostTime = millis();
        if (WiFi.status() == WL_CONNECTED) {
            postData();
        }
    }
}

void TelemetryModule::postData() {
    float weight = scale.getFilteredWeight();
    String timeStr = timeModule.getTimeString();
    
    String payload = "{\"timestamp\":\"" + timeStr + "\",\"weight_g\":" + String(weight, 2) + "}";
    
    // Send HTTP POST
    WiFiClient client;
    HTTPClient http;
    http.begin(client, settings->getServerUrl());
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(payload);
    if (httpCode > 0) {
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
                if (!doc["config"]["sleep_interval_sec"].isNull()) {
                    long sleepSec = doc["config"]["sleep_interval_sec"];
                    settings->setSleepIntervalSec(sleepSec);
                    Logger::info("Sleep interval updated.");
                }
                if (!doc["config"]["timezone_offset_sec"].isNull()) {
                    long tzOffset = doc["config"]["timezone_offset_sec"];
                    settings->setTimezoneOffsetSec(tzOffset);
                    Logger::info("Timezone offset updated.");
                }
                if (!doc["config"]["debug_mode"].isNull()) {
                    bool debugMode = doc["config"]["debug_mode"];
                    settings->setDebugMode(debugMode);
                    Logger::info(debugMode ? "Debug Mode: ON" : "Debug Mode: OFF");
                }
            }
        }
    } else {
        Logger::error("Failed to send telemetry payload.");
    }
    http.end();
}
