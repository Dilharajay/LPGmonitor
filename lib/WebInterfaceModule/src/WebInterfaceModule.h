#ifndef WEB_INTERFACE_MODULE_H
#define WEB_INTERFACE_MODULE_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "SettingsModule.h"
#include "ScaleDriver.h"
#include "GasSensorModule.h"
#include "TimeModule.h"
#include "TerminalCLI.h"

class WebInterfaceModule {
public:
    WebInterfaceModule(ScaleDriver& scaleDriver, GasSensorModule& gasSensor, TimeModule& timeMod);
    void begin(SettingsModule& s, TerminalCLI& cli);
    void update();

private:
    ESP8266WebServer server;
    SettingsModule* settings;
    ScaleDriver& scale;
    GasSensorModule& gasSensor;
    TimeModule& timeModule;
    unsigned long bootTimeMs;
    
    void handleRoot();
    void handleConfig();
    void handleStatus();
};

#endif
