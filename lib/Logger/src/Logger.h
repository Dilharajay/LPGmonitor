#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

enum class LogLevel : uint8_t {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3,
    NONE  = 4   // disables all logging
};

class Logger {
public:
    static void begin(bool enableDebug = false);
    static void setDebugMode(bool mode);
    static bool isDebugMode();
    static void setLogLevel(LogLevel level);
    static LogLevel getLogLevel();

    // ── Levelled one-liner helpers (print tag + message + newline) ──

    template <typename T>
    static void debug(T msg) { log(LogLevel::DEBUG, msg); }

    template <typename T>
    static void info(T msg) { log(LogLevel::INFO, msg); }

    template <typename T>
    static void warn(T msg) { log(LogLevel::WARN, msg); }

    template <typename T>
    static void error(T msg) { log(LogLevel::ERROR, msg); }

    // ── Overloads that accept a format / base parameter ──

    template <typename T, typename U>
    static void debug(T msg, U fmt) { log(LogLevel::DEBUG, msg, fmt); }

    template <typename T, typename U>
    static void info(T msg, U fmt) { log(LogLevel::INFO, msg, fmt); }

    template <typename T, typename U>
    static void warn(T msg, U fmt) { log(LogLevel::WARN, msg, fmt); }

    template <typename T, typename U>
    static void error(T msg, U fmt) { log(LogLevel::ERROR, msg, fmt); }

    // ── Raw print (no tag, no newline) — useful for building composite lines ──

    template <typename T>
    static void raw(T msg) { Serial.print(msg); }

    template <typename T, typename U>
    static void raw(T msg, U fmt) { Serial.print(msg, fmt); }

    static void rawln() { Serial.println(); }

    template <typename T>
    static void rawln(T msg) { Serial.println(msg); }

    template <typename T, typename U>
    static void rawln(T msg, U fmt) { Serial.println(msg, fmt); }

    // ── Kept for backward-compat — maps to debug level ──

    template <typename T>
    static void debugln(T msg) { log(LogLevel::DEBUG, msg); }

    template <typename T, typename U>
    static void debugln(T msg, U fmt) { log(LogLevel::DEBUG, msg, fmt); }

    // ── Kept for backward-compat — maps to info level ──

    template <typename T>
    static void infoln(T msg) { log(LogLevel::INFO, msg); }

    template <typename T, typename U>
    static void infoln(T msg, U fmt) { log(LogLevel::INFO, msg, fmt); }

private:
    static bool     debugMode;
    static LogLevel minLevel;

    // Print the tag prefix:  "[  INFO ] (  1234) "
    static void printPrefix(LogLevel level);

    // Core log: prefix + message + newline
    template <typename T>
    static void log(LogLevel level, T msg) {
        if (level < minLevel) return;
        printPrefix(level);
        Serial.println(msg);
    }

    template <typename T, typename U>
    static void log(LogLevel level, T msg, U fmt) {
        if (level < minLevel) return;
        printPrefix(level);
        Serial.println(msg, fmt);
    }
};

#endif
