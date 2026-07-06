#include "MqttModule.h"

MqttModule::MqttModule(ScaleDriver& scale, GasSensorModule& gasSensor, TimeModule& time)
    : _scale(scale), _gasSensor(gasSensor), _time(time), _settings(nullptr), _lastPublishMs(0), _lastReconnectAttemptMs(0), _reconnectDelayMs(2000) {
}

void MqttModule::begin(SettingsModule& settings) {
    _settings = &settings;
    _wifiClient.setTimeout(2000);
    _client.setClient(_wifiClient);
    _client.setBufferSize(512);
    _client.setServer(_settings->getMqttBroker(), _settings->getMqttPort());
}

void MqttModule::update() {
    if (!_settings) return;
    if (WiFi.status() != WL_CONNECTED || !_settings->isTelemetryEnabled()) {
        return;
    }

    if (!_client.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttemptMs > _reconnectDelayMs) {
            _lastReconnectAttemptMs = now;
            reconnect();
        }
    } else {
        _client.loop();
        
        unsigned long now = millis();
        if (now - _lastPublishMs > 30000) {
            _lastPublishMs = now;
            publishTelemetry();
        }
    }
}

void MqttModule::reconnect() {
    if (WiFi.status() == WL_CONNECTED) {
        const char* user = _settings->getMqttUser();
        const char* pass = _settings->getMqttPassword();
        
        char clientId[24];
        snprintf(clientId, sizeof(clientId), "LPG_%08X", ESP.getChipId());
        
        bool connected = false;
        if (strlen(user) > 0) {
            connected = _client.connect(clientId, user, pass);
        } else {
            connected = _client.connect(clientId);
        }
        
        if (connected) {
            // connected successfully
            _reconnectDelayMs = 2000; // reset backoff on success
        } else {
            // increase delay with cap and small jitter
            _reconnectDelayMs = min(60000UL, _reconnectDelayMs * 2 + (random(0, 1000)));
        }
    }
}

void MqttModule::publishTelemetry() {
    if (!_client.connected()) return;

    // Weight
    float weight = _scale.getFilteredWeight();
    char weightStr[16];
    snprintf(weightStr, sizeof(weightStr), "%.2f", weight);
    _client.publish("lpgmonitor/weight", weightStr);

    // Gas Level
    float emptyWeight = _settings->getEmptyCylinderWeight();
    float fullWeight = _settings->getFullCylinderWeight();
    float gasCapacity = fullWeight - emptyWeight;
    
    float gasPercent = 0.0f;
    if (gasCapacity > 0.0f) {
        float gasRemaining = weight - emptyWeight;
        gasPercent = (gasRemaining / gasCapacity) * 100.0f;
        if (gasPercent < 0.0f) gasPercent = 0.0f;
        if (gasPercent > 100.0f) gasPercent = 100.0f;
    }
    
    char gasPercentStr[16];
    snprintf(gasPercentStr, sizeof(gasPercentStr), "%.1f", gasPercent);
    _client.publish("lpgmonitor/gas_level", gasPercentStr);

    // Gas PPM
    float ppm = _gasSensor.getPPM();
    char ppmStr[16];
    snprintf(ppmStr, sizeof(ppmStr), "%.0f", ppm);
    _client.publish("lpgmonitor/gas_ppm", ppmStr);

    // Status
    if (_gasSensor.isLeakDetected()) {
        _client.publish("lpgmonitor/status", "Leak");
    } else {
        _client.publish("lpgmonitor/status", "Normal");
    }
}
