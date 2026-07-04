#ifndef MQTT_MODULE_H
#define MQTT_MODULE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "SettingsModule.h"
#include "ScaleDriver.h"
#include "GasSensorModule.h"
#include "TimeModule.h"

class MqttModule {
public:
    MqttModule(ScaleDriver& scale, GasSensorModule& gasSensor, TimeModule& time);
    void begin(SettingsModule& settings);
    void update();

private:
    void reconnect();
    void publishTelemetry();

    ScaleDriver& _scale;
    GasSensorModule& _gasSensor;
    TimeModule& _time;
    SettingsModule* _settings;

    WiFiClient _wifiClient;
    PubSubClient _client;

    unsigned long _lastPublishMs;
    unsigned long _lastReconnectAttemptMs;
};

#endif // MQTT_MODULE_H
