#include "OTAManager.h"
#include "Logger.h"
#include <ArduinoOTA.h>

namespace OTA
{

void begin(const char* hostname,
           const char* password)
{
    ArduinoOTA.setHostname(hostname);

    if(password != nullptr)
    {
        ArduinoOTA.setPassword(password);
    }

    ArduinoOTA.onStart([]()
    {
        Logger::info("OTA Update Started");
    });

    ArduinoOTA.onEnd([]()
    {
        Logger::info("OTA Update Finished");
    });

    ArduinoOTA.onProgress([](unsigned int progress,
                             unsigned int total)
    {
        Logger::info("OTA Progress: " + String(progress) + "/" + String(total));
    });

    ArduinoOTA.onError([](ota_error_t error)
    {
        Logger::error("OTA Error: " + String(error));
    });

    ArduinoOTA.begin();

    Logger::info("OTA Ready. Hostname: " + String(hostname));
}

void loop()
{
    ArduinoOTA.handle();
}

}