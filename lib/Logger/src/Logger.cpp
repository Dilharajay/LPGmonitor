#include "Logger.h"

bool     Logger::debugMode = false;
LogLevel Logger::minLevel  = LogLevel::INFO;  // show INFO and above by default
static char logRing[2048];
static size_t logHead = 0;
static size_t logLen = 0;

void Logger::begin(bool enableDebug) {
    debugMode = enableDebug;
    minLevel  = enableDebug ? LogLevel::DEBUG : LogLevel::INFO;
}

void Logger::setDebugMode(bool mode) {
    debugMode = mode;
    minLevel  = mode ? LogLevel::DEBUG : LogLevel::INFO;
}

bool Logger::isDebugMode() {
    return debugMode;
}

void Logger::setLogLevel(LogLevel level) {
    minLevel = level;
    debugMode = (level <= LogLevel::DEBUG);
}

LogLevel Logger::getLogLevel() {
    return minLevel;
}

String Logger::getLogBuffer() {
    String out;
    out.reserve(logLen);
    for (size_t i = 0; i < logLen; i++) {
        out += logRing[(logHead + i) % sizeof(logRing)];
    }
    return out;
}

void Logger::appendLog(String s) {
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (logLen < sizeof(logRing)) {
            logRing[(logHead + logLen) % sizeof(logRing)] = c;
            logLen++;
        } else {
            logRing[logHead] = c;
            logHead = (logHead + 1) % sizeof(logRing);
        }
    }
}

// ── Tag prefix with uptime timestamp ──────────────────────────────
//
//   Output format:  [  INFO ] (  1234) Your message here
//
void Logger::printPrefix(LogLevel level) {
    const char* tag;
    switch (level) {
        case LogLevel::DEBUG: tag = " DEBUG "; break;
        case LogLevel::INFO:  tag = " INFO  "; break;
        case LogLevel::WARN:  tag = " WARN  "; break;
        case LogLevel::ERROR: tag = " ERROR "; break;
        default:              tag = " ???   "; break;
    }

    // Timestamp: millis() right-justified in 8-char field
    unsigned long ms = millis();
    char timeBuf[10];
    snprintf(timeBuf, sizeof(timeBuf), "%8lu", ms);

    Serial.print("[");
    Serial.print(tag);
    Serial.print("] (");
    Serial.print(timeBuf);
    Serial.print(") ");
    
    appendLog(String("[") + tag + "] (" + timeBuf + ") ");
}
