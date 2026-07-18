#include "WebInterfaceModule.h"
#include "Logger.h"
#include <ArduinoJson.h>

// ── Dashboard HTML ─────────────────────────────────────────────────
// Designed to match a Home-Assistant-style dark LPG monitor dashboard
// with a slide-out settings panel on the right.

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>LPG Monitor</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
:root{--bg:#0d1117;--card:#161b22;--card2:#1c2333;--accent:#58a6ff;--green:#3fb950;--orange:#f0883e;--red:#f85149;--txt:#e6edf3;--txt2:#8b949e;--border:#30363d;--radius:14px}
body{font-family:'Segoe UI',system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--txt);overflow-x:hidden}

/* ── Header ── */
.header{background:linear-gradient(135deg,#161b22 0%,#1a2332 100%);padding:14px 24px;display:flex;align-items:center;justify-content:space-between;border-bottom:1px solid var(--border)}
.header h1{font-size:18px;font-weight:700;letter-spacing:.5px}
.header h1 span{color:var(--accent);font-weight:400;font-size:12px;display:block;margin-top:2px}
.gear-btn{background:none;border:none;color:var(--txt2);font-size:22px;cursor:pointer;padding:8px;border-radius:8px;transition:.2s}
.gear-btn:hover{background:var(--card2);color:var(--txt)}

/* ── Grid Layout ── */
.grid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:14px;padding:18px 24px}
.grid .span2{grid-column:span 2}
.grid .span3{grid-column:span 3}

/* ── Cards ── */
.card{background:var(--card);border:1px solid var(--border);border-radius:var(--radius);padding:18px;position:relative;transition:transform .15s,box-shadow .15s}
.card:hover{transform:translateY(-2px);box-shadow:0 8px 24px rgba(0,0,0,.3)}
.card-title{font-size:11px;text-transform:uppercase;letter-spacing:1.2px;color:var(--txt2);margin-bottom:10px;font-weight:600}
.card-value{font-size:36px;font-weight:700;line-height:1.1}
.card-sub{font-size:13px;color:var(--txt2);margin-top:4px}

/* ── Cylinder Card ── */
.cyl-card{display:flex;align-items:center;gap:18px}
.cyl-icon{width:60px;height:80px;position:relative}
.cyl-icon svg{width:100%;height:100%}
.cyl-info .card-value{color:var(--accent)}

/* ── Gauge Card ── */
.gauge-wrap{display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:140px}
.gauge{position:relative;width:120px;height:120px}
.gauge svg{transform:rotate(-90deg)}
.gauge-text{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center}
.gauge-text .pct{font-size:32px;font-weight:700}
.gauge-text .lbl{font-size:11px;color:var(--txt2)}

/* ── Gas Leak Card ── */
.ppm-val{font-size:42px;font-weight:700}
.ppm-val.normal{color:var(--green)}
.ppm-val.warning{color:var(--orange)}
.ppm-val.danger{color:var(--red)}

/* ── Status Badge ── */
.status-badge{display:inline-flex;align-items:center;gap:6px;padding:6px 14px;border-radius:20px;font-size:13px;font-weight:600}
.status-badge.ok{background:rgba(63,185,80,.15);color:var(--green)}
.status-badge.warn{background:rgba(240,136,62,.15);color:var(--orange)}
.status-badge.alert{background:rgba(248,81,73,.15);color:var(--red)}
.status-dot{width:8px;height:8px;border-radius:50%;animation:pulse 2s infinite}
.status-badge.ok .status-dot{background:var(--green)}
.status-badge.warn .status-dot{background:var(--orange)}
.status-badge.alert .status-dot{background:var(--red)}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.4}}

/* ── Small Info Cards ── */
.info-row{display:grid;grid-template-columns:1fr 1fr 1fr;gap:14px}
.info-card{display:flex;align-items:center;gap:12px}
.info-icon{width:36px;height:36px;border-radius:10px;display:flex;align-items:center;justify-content:center;font-size:16px}
.info-icon.wifi{background:rgba(88,166,255,.15);color:var(--accent)}
.info-icon.up{background:rgba(63,185,80,.15);color:var(--green)}
.info-icon.mem{background:rgba(188,143,255,.15);color:#bc8fff}
.info-val{font-size:18px;font-weight:700}
.info-lbl{font-size:11px;color:var(--txt2)}

/* ── Progress Bar (Gas Level) ── */
.gas-bar{height:10px;background:var(--border);border-radius:5px;margin-top:10px;overflow:hidden}
.gas-bar-fill{height:100%;border-radius:5px;transition:width .6s ease,background .4s}

/* ── Logs ── */
.logs-box{background:#0a0e14;border:1px solid var(--border);border-radius:10px;padding:12px;font-family:'Cascadia Code','Fira Code',monospace;font-size:11px;height:180px;overflow-y:auto;white-space:pre-wrap;color:var(--txt2);line-height:1.5}

/* ── Settings Panel ── */
.overlay{position:fixed;inset:0;background:rgba(0,0,0,.5);z-index:99;opacity:0;pointer-events:none;transition:opacity .3s}
.overlay.open{opacity:1;pointer-events:auto}
.settings-panel{position:fixed;top:0;right:-400px;width:380px;height:100%;background:var(--card);border-left:1px solid var(--border);z-index:100;transition:right .35s ease;overflow-y:auto;display:flex;flex-direction:column}
.settings-panel.open{right:0}
.sp-header{padding:18px 20px;border-bottom:1px solid var(--border);display:flex;align-items:center;justify-content:space-between}
.sp-header h2{font-size:16px}
.sp-close{background:none;border:none;color:var(--txt2);font-size:20px;cursor:pointer;padding:4px 8px;border-radius:6px}
.sp-close:hover{background:var(--card2);color:var(--txt)}
.sp-body{padding:20px;flex:1}
.sp-section{margin-bottom:20px}
.sp-section h3{font-size:12px;text-transform:uppercase;letter-spacing:1px;color:var(--accent);margin-bottom:12px;font-weight:600}
.field{margin-bottom:14px}
.field label{display:block;font-size:12px;color:var(--txt2);margin-bottom:4px;font-weight:500}
.field input,.field select{width:100%;padding:9px 12px;background:var(--bg);border:1px solid var(--border);border-radius:8px;color:var(--txt);font-size:13px;outline:none;transition:border .2s}
.field input:focus{border-color:var(--accent)}
.sp-footer{padding:16px 20px;border-top:1px solid var(--border)}
.btn{width:100%;padding:10px;border:none;border-radius:8px;font-size:13px;font-weight:600;cursor:pointer;transition:.2s}
.btn-primary{background:var(--accent);color:#fff}
.btn-primary:hover{background:#79c0ff}
.btn-tare{background:var(--green);color:#fff;margin-top:8px}
.btn-tare:hover{background:#56d364}

/* ── Responsive ── */
@media(max-width:768px){
  .grid{grid-template-columns:1fr;gap:12px;padding:12px}
  .grid .span2,.grid .span3{grid-column:span 1}
  .info-row{grid-template-columns:1fr}
  .settings-panel{width:100%;right:-100%}
}
</style>
</head>
<body>

<!-- ── Header ── -->
<div class="header">
  <h1>&#x1F525; Smart LPG Monitor
    <span>ESP8266 &bull; HX711 (Weight) &bull; MQ-6 (Gas)</span>
  </h1>
  <button class="gear-btn" onclick="toggleSettings()" title="Settings">&#9881;</button>
</div>

<!-- ── Main Dashboard Grid ── -->
<div class="grid">

  <!-- Gas Cylinder Weight -->
  <div class="card">
    <div class="card-title">Gas Cylinder Weight</div>
    <div class="cyl-card">
      <div class="cyl-icon">
        <svg viewBox="0 0 60 80" fill="none"><rect x="15" y="10" width="30" height="60" rx="6" fill="#f85149" opacity=".85"/><rect x="22" y="4" width="16" height="8" rx="3" fill="#8b949e"/><rect x="26" y="0" width="8" height="6" rx="2" fill="#58a6ff"/><rect x="18" y="20" width="24" height="4" rx="1" fill="#fff" opacity=".15"/></svg>
      </div>
      <div class="cyl-info">
        <div class="card-value" id="weightKg">--</div>
        <div class="card-sub">kg</div>
        <div class="card-sub" style="margin-top:2px">Remaining <strong id="remainPct2">--%</strong></div>
      </div>
    </div>
    <div class="gas-bar"><div class="gas-bar-fill" id="gasBar" style="width:0%;background:var(--green)"></div></div>
    <div style="display:flex;justify-content:space-between;margin-top:4px"><span class="card-sub">Empty</span><span class="card-sub" id="fullLabel">Full (20kg)</span></div>
  </div>

  <!-- Gas Level Gauge -->
  <div class="card">
    <div class="card-title">Gas Level</div>
    <div class="gauge-wrap">
      <div class="gauge">
        <svg viewBox="0 0 120 120" width="120" height="120">
          <circle cx="60" cy="60" r="52" fill="none" stroke="#30363d" stroke-width="10"/>
          <circle id="gaugeArc" cx="60" cy="60" r="52" fill="none" stroke="var(--green)" stroke-width="10" stroke-linecap="round" stroke-dasharray="0 327" />
        </svg>
        <div class="gauge-text">
          <span class="pct" id="gaugePct">--%</span>
          <span class="lbl">Remaining</span>
        </div>
      </div>
    </div>
  </div>

  <!-- Gas Leakage -->
  <div class="card" style="display:flex;flex-direction:column;justify-content:space-between">
    <div>
      <div class="card-title">Gas Leakage (MQ-6)</div>
      <div class="ppm-val normal" id="ppmVal">-- <span style="font-size:18px;font-weight:400">ppm</span></div>
    </div>
    <div style="margin-top:auto;padding-top:12px">
      <div class="card-sub">Status</div>
      <div class="status-badge ok" id="statusBadge">
        <div class="status-dot"></div>
        <span id="statusText">Normal</span>
      </div>
    </div>
  </div>

  <!-- Wi-Fi / Uptime / Free Heap -->
  <div class="card span3">
    <div class="info-row">
      <div class="info-card">
        <div class="info-icon wifi">&#x1F4F6;</div>
        <div><div class="info-val" id="rssiVal">--</div><div class="info-lbl">Wi-Fi Signal (dBm)</div></div>
      </div>
      <div class="info-card">
        <div class="info-icon up">&#x23F1;</div>
        <div><div class="info-val" id="uptimeVal">--</div><div class="info-lbl">Uptime</div></div>
      </div>
      <div class="info-card">
        <div class="info-icon mem">&#x1F9E0;</div>
        <div><div class="info-val" id="heapVal">--</div><div class="info-lbl">Free Memory</div></div>
      </div>
    </div>
  </div>

  <!-- Live Logs -->
  <div class="card span3">
    <div class="card-title">Live Logs</div>
    <div class="logs-box" id="logsBox">Connecting...</div>
  </div>

</div>

<!-- ── Settings Overlay ── -->
<div class="overlay" id="overlay" onclick="toggleSettings()"></div>
<div class="settings-panel" id="settingsPanel">
  <div class="sp-header">
    <h2>&#9881; Settings</h2>
    <button class="sp-close" onclick="toggleSettings()">&times;</button>
  </div>
  <div class="sp-body">
    <div class="sp-section">
      <h3>&#x1F525; Cylinder Configuration</h3>
      <div class="field"><label>Full Cylinder Weight (g)</label><input type="number" id="cfgFullWeight" placeholder="20000"></div>
      <div class="field"><label>Empty Cylinder Weight (g)</label><input type="number" id="cfgEmptyWeight" placeholder="6500"></div>
    </div>
    <div class="sp-section">
      <h3>&#x1F4A8; Gas Sensor</h3>
      <div class="field"><label>Leak Detection Threshold (ppm)</label><input type="number" id="cfgGasThreshold" placeholder="700"></div>
    </div>
    <div class="sp-section">
      <h3>&#x1F310; Network &amp; Time</h3>
      <div class="field"><label>WiFi SSID</label><input type="text" id="cfgSsid"></div>
      <div class="field"><label>WiFi Password (leave blank to keep)</label><input type="password" id="cfgWifiPwd"></div>
      <div class="field"><label>NTP Server</label><input type="text" id="cfgNtp"></div>
      <div class="field"><label>Timezone Offset (hours from UTC)</label><input type="number" step="0.5" id="cfgTz"></div>
    </div>
    <div class="sp-section">
      <h3>&#x1F4E1; MQTT Settings</h3>
      <div class="field"><label>MQTT Broker</label><input type="text" id="cfgMqttBroker"></div>
      <div class="field"><label>MQTT Port</label><input type="number" id="cfgMqttPort"></div>
      <div class="field"><label>MQTT Username</label><input type="text" id="cfgMqttUser"></div>
      <div class="field"><label>MQTT Password (leave blank to keep)</label><input type="password" id="cfgMqttPwd"></div>
    </div>
    <div class="sp-section">
      <h3>&#x1F512; Security</h3>
      <div class="field"><label>OTA Password (leave blank to keep)</label><input type="password" id="cfgOtaPwd"></div>
    </div>
    <div class="sp-section">
      <h3>&#x2696; Scale</h3>
      <div class="field"><label>Tare Offset</label><input type="number" id="cfgTare"></div>
    </div>
  </div>
  <div class="sp-footer">
    <button class="btn btn-primary" onclick="saveConfig()">Save Configuration</button>
    <button class="btn btn-tare" onclick="tareScale()">Tare Scale (Zero)</button>
  </div>
</div>

<script>
// ── Settings panel toggle ──
function toggleSettings(){
  document.getElementById('settingsPanel').classList.toggle('open');
  document.getElementById('overlay').classList.toggle('open');
}

// ── Format uptime ──
function fmtUp(ms){
  var s=Math.floor(ms/1000),d=Math.floor(s/86400);s%=86400;
  var h=Math.floor(s/3600);s%=3600;var m=Math.floor(s/60);
  return (d>0?d+'d ':'')+(h>0?h+'h ':'')+(m>0?m+'m':'<1m');
}

// ── Gauge color helper ──
function lvlColor(p){return p>30?'var(--green)':p>10?'var(--orange)':'var(--red)';}

// ── Config loaded flag ──
var cfgLoaded=false;

// ── Fetch status ──
function fetchStatus(){
  fetch('/api/status').then(function(r){return r.json()}).then(function(d){
    // Weight in kg
    var wKg=(d.weight/1000).toFixed(2);
    document.getElementById('weightKg').innerText=wKg;

    // Gas percentage
    var full=d.full_cyl_weight||20000, empty=d.empty_cyl_weight||6500;
    var gasW=d.weight-empty;
    if(gasW<0)gasW=0;
    var maxGas=full-empty;
    var pct=maxGas>0?Math.round((gasW/maxGas)*100):0;
    if(pct>100)pct=100;if(pct<0)pct=0;
    document.getElementById('remainPct2').innerText=pct+'%';
    document.getElementById('gaugePct').innerText=pct+'%';
    document.getElementById('fullLabel').innerText='Full ('+(full/1000).toFixed(1)+'kg)';

    // Gauge arc
    var circ=2*Math.PI*52;
    var arc=(pct/100)*circ;
    var g=document.getElementById('gaugeArc');
    g.setAttribute('stroke-dasharray',arc+' '+circ);
    g.setAttribute('stroke',lvlColor(pct));

    // Gas bar
    var bar=document.getElementById('gasBar');
    bar.style.width=pct+'%';
    bar.style.background=lvlColor(pct);

    // MQ-6
    var ppm=d.gas_ppm||0;
    var threshold=d.gas_threshold||700;
    var pe=document.getElementById('ppmVal');
    pe.innerHTML=ppm+' <span style="font-size:18px;font-weight:400">ppm</span>';
    pe.className='ppm-val '+(ppm>=threshold?'danger':ppm>=threshold*0.7?'warning':'normal');

    // Status badge
    var sb=document.getElementById('statusBadge');
    var st=document.getElementById('statusText');
    if(ppm>=threshold){sb.className='status-badge alert';st.innerText='GAS LEAK!';}
    else if(pct<10){sb.className='status-badge warn';st.innerText='Low Gas';}
    else{sb.className='status-badge ok';st.innerText='Normal';}

    // Info row
    document.getElementById('rssiVal').innerText=(d.rssi||0)+' dBm';
    document.getElementById('uptimeVal').innerText=fmtUp(d.uptime_ms||0);
    document.getElementById('heapVal').innerText=((d.free_heap||0)/1024).toFixed(1)+' KB';

    // Logs
    var lb=document.getElementById('logsBox');
    lb.innerText=d.logs||'';
    lb.scrollTop=lb.scrollHeight;

    // Load config into settings panel once
    if(!cfgLoaded){
      document.getElementById('cfgFullWeight').value=d.full_cyl_weight||20000;
      document.getElementById('cfgEmptyWeight').value=d.empty_cyl_weight||6500;
      document.getElementById('cfgGasThreshold').value=d.gas_threshold||700;
      document.getElementById('cfgNtp').value=d.ntp_server||'';
      document.getElementById('cfgTz').value=((d.tz_offset||0)/3600).toFixed(1);
      document.getElementById('cfgTare').value=d.tare_offset||0;
      document.getElementById('cfgSsid').value=d.ssid||'';
      document.getElementById('cfgMqttBroker').value=d.mqtt_broker||'';
      document.getElementById('cfgMqttPort').value=d.mqtt_port||1883;
      document.getElementById('cfgMqttUser').value=d.mqtt_user||'';
      cfgLoaded=true;
    }
  }).catch(function(e){console.error(e)});
}

// ── Save config ──
function saveConfig(){
  var data={
    ssid:document.getElementById('cfgSsid').value,
    wifi_pwd:document.getElementById('cfgWifiPwd').value,
    ntp_server:document.getElementById('cfgNtp').value,
    tz_offset:Math.round(parseFloat(document.getElementById('cfgTz').value)*3600),
    tare_offset:parseInt(document.getElementById('cfgTare').value),
    full_cyl_weight:parseFloat(document.getElementById('cfgFullWeight').value),
    empty_cyl_weight:parseFloat(document.getElementById('cfgEmptyWeight').value),
    gas_threshold:parseInt(document.getElementById('cfgGasThreshold').value),
    mqtt_broker:document.getElementById('cfgMqttBroker').value,
    mqtt_port:parseInt(document.getElementById('cfgMqttPort').value),
    mqtt_user:document.getElementById('cfgMqttUser').value,
    mqtt_pwd:document.getElementById('cfgMqttPwd').value,
    ota_pwd:document.getElementById('cfgOtaPwd').value
  };
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'text/plain'},body:JSON.stringify(data)})
  .then(function(r){return r.json()})
  .then(function(){alert('Settings saved!');toggleSettings();})
  .catch(function(e){alert('Error: '+e)});
}

// ── Tare ──
function tareScale(){
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'text/plain'},body:JSON.stringify({action:'tare'})})
  .then(function(){cfgLoaded=false;alert('Scale tared!');})
  .catch(function(e){alert('Error: '+e)});
}

setInterval(fetchStatus,1500);
fetchStatus();
</script>
</body>
</html>
)rawliteral";

// ── Module Implementation ──────────────────────────────────────────

WebInterfaceModule::WebInterfaceModule(ScaleDriver& scaleDriver, GasSensorModule& gasSen, TimeModule& timeMod)
    : server(80), settings(nullptr), scale(scaleDriver), gasSensor(gasSen), timeModule(timeMod), bootTimeMs(0) {}

void WebInterfaceModule::begin(SettingsModule& s, TerminalCLI& cli) {
    settings = &s;
    bootTimeMs = millis();

    if (WiFi.status() == WL_CONNECTED) {
        Logger::info(F("Web Interface Server IP: "));
        Logger::info(WiFi.localIP().toString().c_str());

        server.on("/", HTTP_GET, [this]() { this->handleRoot(); });
        server.on("/api/status", HTTP_GET, [this]() { this->handleStatus(); });
        server.on("/api/config", HTTP_POST, [this]() { this->handleConfig(); });
        
        server.begin();
        Logger::info(F("HTTP server started"));
    } else {
        Logger::error(F("WebInterface: WiFi not connected, server not started."));
    }
}

void WebInterfaceModule::update() {
    server.handleClient();
}

void WebInterfaceModule::handleRoot() {
    if (!server.authenticate("admin", settings->getOtaPassword())) {
        return server.requestAuthentication();
    }
    server.send_P(200, "text/html", htmlPage);
}

void WebInterfaceModule::handleStatus() {
    if (!server.authenticate("admin", settings->getOtaPassword())) {
        return server.requestAuthentication();
    }
    JsonDocument doc;
    doc["weight"] = scale.getFilteredWeight();
    doc["time"] = timeModule.getTimeString();
    doc["ssid"] = settings->getSSID();
    doc["ntp_server"] = settings->getNtpServer();
    doc["tz_offset"] = settings->getTimezoneOffsetSec();
    doc["mqtt_broker"] = settings->getMqttBroker();
    doc["mqtt_port"] = settings->getMqttPort();
    doc["mqtt_user"] = settings->getMqttUser();
    doc["tare_offset"] = settings->getTareOffset();
    doc["full_cyl_weight"] = settings->getFullCylinderWeight();
    doc["empty_cyl_weight"] = settings->getEmptyCylinderWeight();
    doc["gas_ppm"] = gasSensor.getPPM();
    doc["gas_raw"] = gasSensor.getRawValue();
    doc["gas_threshold"] = settings->getGasLeakThreshold();
    doc["gas_leak"] = gasSensor.isLeakDetected();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime_ms"] = millis() - bootTimeMs;
    doc["free_heap"] = ESP.getFreeHeap();
    doc["logs"] = Logger::getLogBuffer();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebInterfaceModule::handleConfig() {
    if (!server.authenticate("admin", settings->getOtaPassword())) {
        return server.requestAuthentication();
    }
    
    String body;
    if (server.hasArg("plain")) {
        body = server.arg("plain");
    } else if (server.args() > 0) {
        body = server.arg(0);
    } else {
        server.send(400, "application/json", "{\"error\":\"Body not received\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    // Tare action
    if (doc["action"].is<const char*>() && String(doc["action"].as<const char*>()) == "tare") {
        scale.performTare();
        settings->setTareOffset(scale.getTareOffset());
        server.send(200, "application/json", "{\"status\":\"success\"}");
        return;
    }

    // Config updates
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
    if (doc["full_cyl_weight"].is<float>()) {
        float val = doc["full_cyl_weight"].as<float>();
        if (val > 0) settings->setFullCylinderWeight(val);
    }
    if (doc["empty_cyl_weight"].is<float>()) {
        float val = doc["empty_cyl_weight"].as<float>();
        if (val > 0) settings->setEmptyCylinderWeight(val);
    }
    if (doc["gas_threshold"].is<int>()) {
        int val = doc["gas_threshold"].as<int>();
        if (val > 0) {
            settings->setGasLeakThreshold(val);
            gasSensor.setLeakThreshold(val);
        }
    }
    
    // New Settings
    if (doc["ssid"].is<const char*>()) {
        settings->setSSID(doc["ssid"].as<const char*>());
    }
    if (doc["wifi_pwd"].is<const char*>()) {
        String pwd = doc["wifi_pwd"].as<const char*>();
        if (pwd.length() > 0) {
            settings->setPassword(pwd.c_str());
        }
    }
    if (doc["mqtt_broker"].is<const char*>()) {
        settings->setMqttBroker(doc["mqtt_broker"].as<const char*>());
    }
    if (doc["mqtt_port"].is<int>()) {
        settings->setMqttPort(doc["mqtt_port"].as<int>());
    }
    if (doc["mqtt_user"].is<const char*>()) {
        settings->setMqttUser(doc["mqtt_user"].as<const char*>());
    }
    if (doc["mqtt_pwd"].is<const char*>()) {
        String pwd = doc["mqtt_pwd"].as<const char*>();
        if (pwd.length() > 0) {
            settings->setMqttPassword(pwd.c_str());
        }
    }
    if (doc["ota_pwd"].is<const char*>()) {
        String pwd = doc["ota_pwd"].as<const char*>();
        if (pwd.length() > 0) {
            settings->setOtaPassword(pwd.c_str());
        }
    }

    server.send(200, "application/json", "{\"status\":\"success\"}");
}
