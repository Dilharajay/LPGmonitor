#include <Arduino.h>
#include <unity.h>
#include "SettingsModule.h"
#include "TerminalCLI.h"
#include "Logger.h"

TerminalCLI cli;
SettingsModule settings;

void setUp(void) {
    // This is run before EACH test
}

void tearDown(void) {
    // This is run after EACH test
}

void test_telemetry_toggle(void) {
    settings.setTelemetryEnabled(true);
    TEST_ASSERT_TRUE(settings.isTelemetryEnabled());
    
    settings.setTelemetryEnabled(false);
    TEST_ASSERT_FALSE(settings.isTelemetryEnabled());
}

void test_string_saving(void) {
    settings.setNtpServer("test.ntp.org");
    TEST_ASSERT_EQUAL_STRING("test.ntp.org", settings.getNtpServer());
    
    settings.setServerUrl("http://localhost/test");
    TEST_ASSERT_EQUAL_STRING("http://localhost/test", settings.getServerUrl());
}

void test_tare_offset(void) {
    settings.setTareOffset(987654321);
    TEST_ASSERT_EQUAL_INT32(987654321, settings.getTareOffset());
    
    settings.setTareOffset(-123456);
    TEST_ASSERT_EQUAL_INT32(-123456, settings.getTareOffset());
}

void setup() {
    delay(2000); // Wait for board to stabilize
    
    Logger::begin(false); // Disable debug logs to keep test output clean
    settings.begin(cli);  // Initialize EEPROM and Settings

    UNITY_BEGIN();
    RUN_TEST(test_telemetry_toggle);
    RUN_TEST(test_string_saving);
    RUN_TEST(test_tare_offset);
    UNITY_END();
}

void loop() {
    delay(100);
}
