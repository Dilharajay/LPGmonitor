#include "OTAManager.h"
#include "Logger.h"
#include <ArduinoOTA.h>
#include "DisplayModule.h"

extern DisplayModule displayModule;

namespace OTA
{

void begin(const char* hostname,
           const char* password)
{
    ArduinoOTA.setHostname(hostname);

    if (password == nullptr || strlen(password) == 0) {
        Logger::warn(F("⚠ OTA password not set! OTA DISABLED for safety."));
        Logger::warn(F("  Set one with: set_ota_pwd <password>"));
        return; // Don't start OTA without authentication
    }
    ArduinoOTA.setPassword(password);

    ArduinoOTA.onStart([]()
    {
        Logger::info(F("OTA Update Started"));
    });

    ArduinoOTA.onEnd([]()
    {
        Logger::info(F("OTA Update Finished"));
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
        static unsigned int lastPercent = 0;
        unsigned int percent = (progress * 100) / total;
        if (percent / 10 != lastPercent / 10) {
            lastPercent = percent;
            char buf[24];
            snprintf(buf, sizeof(buf), "OTA: %u%%", percent);
            Logger::info(buf);
        }
        displayModule.showOtaScreen(percent);
    });

    ArduinoOTA.onError([](ota_error_t error)
    {
        Logger::error(F("OTA Error: ") + String(error));
    });

    ArduinoOTA.begin();

    Logger::info(F("OTA Ready. Hostname: ") + String(hostname));
}

void loop()
{
    ArduinoOTA.handle();
}

}