#include "WebInterfaceModule.h"
#include "Logger.h"
#include <ArduinoJson.h>

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Scale Dashboard</title>
  <style>
    body { font-family: sans-serif; background: #222; color: #fff; padding: 20px; }
    .container { max-width: 800px; margin: auto; background: #333; padding: 20px; border-radius: 8px; }
    h1 { text-align: center; color: #4CAF50; }
    .display { font-size: 60px; text-align: center; margin: 20px 0; color: #4CAF50; font-weight: bold; }
    .form-group { margin-bottom: 15px; }
    label { display: block; margin-bottom: 5px; }
    input { width: 100%; padding: 8px; box-sizing: border-box; background: #444; color: white; border: 1px solid #555; }
    button { background: #4CAF50; color: white; border: none; padding: 10px 20px; cursor: pointer; border-radius: 4px; }
    button:hover { background: #45a049; }
    .logs { background: #111; padding: 10px; font-family: monospace; height: 200px; overflow-y: scroll; white-space: pre-wrap; font-size: 12px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Digital Scale Dashboard</h1>
    <div class="display" id="weightDisplay">-- g</div>
    <div style="text-align: center; color: #aaa; margin-bottom: 20px;" id="timeDisplay">Syncing...</div>
    
    <h3>Configuration</h3>
    <div class="form-group">
      <label>NTP Server</label>
      <input type="text" id="ntpServer">
    </div>
    <div class="form-group">
      <label>Timezone Offset (Seconds)</label>
      <input type="number" id="tzOffset">
    </div>
    <div class="form-group">
      <label>Tare Offset</label>
      <input type="number" id="tareOffset">
    </div>
    <button onclick="saveConfig()">Save Configuration</button>
    <button onclick="tareScale()" style="background: #2196F3; margin-left: 10px;">Tare Scale</button>
    
    <h3 style="margin-top: 30px;">Live Logs</h3>
    <div class="logs" id="logsBox"></div>
  </div>

  <script>
    function fetchStatus() {
      fetch('/api/status')
        .then(res => res.json())
        .then(data => {
          document.getElementById('weightDisplay').innerText = data.weight.toFixed(2) + ' g';
          document.getElementById('timeDisplay').innerText = data.time;
          
          let logsBox = document.getElementById('logsBox');
          logsBox.innerText = data.logs;
          
          if (!document.getElementById('ntpServer').dataset.loaded) {
            document.getElementById('ntpServer').value = data.ntp_server;
            document.getElementById('tzOffset').value = data.tz_offset;
            document.getElementById('tareOffset').value = data.tare_offset;
            document.getElementById('ntpServer').dataset.loaded = 'true';
          }
        })
        .catch(err => console.error(err));
    }

    function saveConfig() {
      const data = {
        ntp_server: document.getElementById('ntpServer').value,
        tz_offset: parseInt(document.getElementById('tzOffset').value),
        tare_offset: parseInt(document.getElementById('tareOffset').value)
      };
      
      fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data)
      }).then(res => res.json()).then(res => alert('Saved successfully!'));
    }

    function tareScale() {
      fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ action: 'tare' })
      }).then(() => {
        document.getElementById('ntpServer').dataset.loaded = ''; // Force reload of tare value
      });
    }

    setInterval(fetchStatus, 1000);
    fetchStatus();
  </script>
</body>
</html>
)rawliteral";

WebInterfaceModule::WebInterfaceModule(ScaleDriver& scaleDriver, TimeModule& timeMod)
    : server(80), settings(nullptr), scale(scaleDriver), timeModule(timeMod) {}

void WebInterfaceModule::begin(SettingsModule& s, TerminalCLI& cli) {
    settings = &s;

    if (WiFi.status() != WL_CONNECTED) {
        Logger::info("WebInterface: Connecting to WiFi...");
        WiFi.begin(settings->getSSID(), settings->getPassword());
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Logger::raw(".");
            attempts++;
        }
        Logger::rawln();
    }

    if (WiFi.status() == WL_CONNECTED) {
        Logger::info("Web Server IP: ");
        Logger::info(WiFi.localIP().toString().c_str());

        server.on("/", HTTP_GET, [this]() { this->handleRoot(); });
        server.on("/api/status", HTTP_GET, [this]() { this->handleStatus(); });
        server.on("/api/config", HTTP_POST, [this]() { this->handleConfig(); });
        
        server.begin();
        Logger::info("HTTP server started");
    } else {
        Logger::error("WebInterface: WiFi not connected, server not started.");
    }
}

void WebInterfaceModule::update() {
    server.handleClient();
}

void WebInterfaceModule::handleRoot() {
    server.send(200, "text/html", htmlPage);
}

void WebInterfaceModule::handleStatus() {
    JsonDocument doc;
    doc["weight"] = scale.getFilteredWeight();
    doc["time"] = timeModule.getTimeString();
    doc["ntp_server"] = settings->getNtpServer();
    doc["tz_offset"] = settings->getTimezoneOffsetSec();
    doc["tare_offset"] = settings->getTareOffset();
    doc["logs"] = Logger::getLogBuffer();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebInterfaceModule::handleConfig() {
    if (server.hasArg("plain") == false) {
        server.send(400, "application/json", "{\"error\":\"Body not received\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    if (doc["action"].is<const char*>() && String(doc["action"].as<const char*>()) == "tare") {
        scale.performTare();
        settings->setTareOffset(scale.getTareOffset());
        server.send(200, "application/json", "{\"status\":\"success\"}");
        return;
    }

    if (doc["ntp_server"].is<const char*>()) {
        settings->setNtpServer(doc["ntp_server"].as<const char*>());
    }
    if (doc["tz_offset"].is<long>()) {
        settings->setTimezoneOffsetSec(doc["tz_offset"].as<long>());
    }
    if (doc["tare_offset"].is<long>()) {
        settings->setTareOffset(doc["tare_offset"].as<long>());
        scale.setTareOffset(doc["tare_offset"].as<long>());
    }

    server.send(200, "application/json", "{\"status\":\"success\"}");
}
