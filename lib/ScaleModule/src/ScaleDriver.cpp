#include "ScaleDriver.h"
#include "Logger.h"

// ── Constructor ────────────────────────────────────────────────────
ScaleDriver::ScaleDriver()
    : calibrationFactor(1.0f),
      medianIndex(0),
      medianCount(0),
      emaValue(0.0f),
      emaInitialized(false)
{
    for (int i = 0; i < Config::MEDIAN_WINDOW; i++) {
        medianBuffer[i] = 0.0f;
    }
}

// ── Init ───────────────────────────────────────────────────────────
bool ScaleDriver::begin(uint8_t doutPin, uint8_t sckPin, float calFactor, long savedOffset) {
    scale.begin(doutPin, sckPin);
    calibrationFactor = calFactor;

    Logger::debug(F("HX711 Started"));
    Logger::info(F("Remove all weight..."));
    delay(3000);

    Logger::debug(F("Checking HX711 connection..."));

    if (scale.wait_ready_timeout(2000)) {
        Logger::info(F("HX711 connected! Restoring tare offset from EEPROM."));
        scale.set_offset(savedOffset);
    } else {
        Logger::error(F("HX711 not found!"));
        Logger::error(F("Check wiring! Ensure DT is on D2 and SCK is on D1."));
        // Do not halt the system. Return false so caller can continue in degraded mode.
        return false;
    }

    Logger::debug(F("Setting calibration factor..."));
    scale.set_scale(calibrationFactor);

    Logger::debug(F("Offset = "));
    Logger::debug(scale.get_offset());

    return true;
}

// ── Filter reset ───────────────────────────────────────────────────
void ScaleDriver::resetFilters() {
    medianIndex = 0;
    medianCount = 0;
    emaValue    = 0.0f;
    emaInitialized = false;
    for (int i = 0; i < Config::MEDIAN_WINDOW; i++) {
        medianBuffer[i] = 0.0f;
    }
}

// ── Tare ───────────────────────────────────────────────────────────
long ScaleDriver::performTare() {
    long sum = 0;
    int valid = 0;

    for (int i = 0; i < 10; i++) {
        if (scale.wait_ready_timeout(1000)) {
            sum += scale.read();
            valid++;
        }
        delay(10);
    }

    if (valid > 0) {
        scale.tare(10);
        long offset = scale.get_offset();
        Logger::info(F("Scale Tared. New Offset: "));
        Logger::info(String(offset).c_str());
        resetFilters();
        return offset;
    } else {
        Logger::error(F("Tare failed! HX711 timeout."));
        return 0;
    }
}

long ScaleDriver::getTareOffset() {
    return scale.get_offset();
}

void ScaleDriver::setTareOffset(long offset) {
    scale.set_offset(offset);
    Logger::info(F("Applied tare offset: "));
    Logger::info(String(offset).c_str());
}

// ── Calibration ────────────────────────────────────────────────────
void ScaleDriver::performCalibration(float knownWeight) {
    if (knownWeight <= 0) {
        Logger::warn(F("Invalid weight! Usage: c <weight_in_grams>"));
        return;
    }

    Logger::raw(F("\nCalibrating with known weight: "));
    Logger::raw(knownWeight);
    Logger::rawln(F(" g... please wait."));

    long diffSum = 0;
    int valid = 0;

    for (int i = 0; i < 10; i++) {
        if (scale.wait_ready_timeout(1000)) {
            diffSum += scale.get_value(1);
            valid++;
        }
        delay(10);
    }

    if (valid > 0) {
        long avgDiff = diffSum / valid;
        calibrationFactor = (float)avgDiff / knownWeight;
        scale.set_scale(calibrationFactor);

        Logger::raw(F("Calibration complete! New Calibration Factor: "));
        Logger::rawln(calibrationFactor);
        Logger::info(F("Update 'defaultCalibrationFactor' in main with this value."));

        resetFilters();
    } else {
        Logger::error(F("Calibration failed! HX711 timeout."));
    }
}

// ── Median computation ─────────────────────────────────────────────
//  Copies the buffer, insertion-sorts it (tiny N=5), returns middle.
float ScaleDriver::computeMedian() {
    int n = medianCount;
    float sorted[Config::MEDIAN_WINDOW];
    for (int i = 0; i < n; i++) {
        sorted[i] = medianBuffer[i];
    }

    // Insertion sort (fine for N ≤ 7)
    for (int i = 1; i < n; i++) {
        float key = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }

    // Middle element (for even n, pick lower-middle)
    return sorted[n / 2];
}

// ── Main update loop ───────────────────────────────────────────────
//
//  Pipeline:  raw HX711  →  reject < 0  →  median  →  EMA  →  output
//
void ScaleDriver::update() {
    if (!scale.is_ready()) {
        return;
    }

    float raw = scale.get_units(1);

    // ── Stage 0: reject negative readings ──────────────────────────
    if (raw < 0.0f) {
        Logger::debug(F("Negative reading rejected"));
        return;            // don't feed garbage into the filters
    }

    // ── Stage 1: Median filter ─────────────────────────────────────
    medianBuffer[medianIndex] = raw;
    medianIndex = (medianIndex + 1) % Config::MEDIAN_WINDOW;
    if (medianCount < Config::MEDIAN_WINDOW) {
        medianCount++;
    }

    float medianVal = computeMedian();

    // ── Stage 2: EMA smoothing ─────────────────────────────────────
    if (!emaInitialized) {
        emaValue = medianVal;       // seed with first valid median
        emaInitialized = true;
    } else {
        emaValue = Config::EMA_ALPHA * medianVal + (1.0f - Config::EMA_ALPHA) * emaValue;
    }
}

// ── Getters ────────────────────────────────────────────────────────
float ScaleDriver::getFilteredWeight() {
    return emaValue;
}

float ScaleDriver::getCalibrationFactor() {
    return calibrationFactor;
}
