#include "MqttModule.h"

MqttModule::MqttModule(ScaleDriver& scale, GasSensorModule& gasSensor, TimeModule& time)
    : _scale(scale), _gasSensor(gasSensor), _time(time), _settings(nullptr), _lastPublishMs(0), _lastReconnectAttemptMs(0) {
}

void MqttModule::begin(SettingsModule& settings) {
    _settings = &settings;
    _client.setClient(_wifiClient);
    _client.setServer(_settings->getMqttBroker(), _settings->getMqttPort());
}

void MqttModule::update() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (!_client.connected()) {
        unsigned long now = millis();
        // Wait 5 seconds before retrying
        if (now - _lastReconnectAttemptMs > 5000) {
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
        
        bool connected = false;
        if (strlen(user) > 0) {
            connected = _client.connect("LPGMonitor", user, pass);
        } else {
            connected = _client.connect("LPGMonitor");
        }
        
        if (connected) {
            // connected successfully
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
