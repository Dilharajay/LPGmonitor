#pragma once

namespace OTA {
    void begin(const char* hostname,
    const char* password = nullptr);
    
    void loop();
}