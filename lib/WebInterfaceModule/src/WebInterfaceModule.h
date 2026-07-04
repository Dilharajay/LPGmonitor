#ifndef WEB_INTERFACE_MODULE_H
#define WEB_INTERFACE_MODULE_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "SettingsModule.h"
#include "ScaleDriver.h"
#include "TimeModule.h"
#include "TerminalCLI.h"

class WebInterfaceModule {
public:
    WebInterfaceModule(ScaleDriver& scaleDriver, TimeModule& timeMod);
    void begin(SettingsModule& s, TerminalCLI& cli);
    void update();

private:
    ESP8266WebServer server;
    SettingsModule* settings;
    ScaleDriver& scale;
    TimeModule& timeModule;
    
    void handleRoot();
    void handleConfig();
    void handleStatus();
    void handleLogs();
};

#endif
