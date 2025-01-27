#include "BatteryManager.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h> // For the pow function

extern Adafruit_NeoPixel mainLed;

BatteryManager::BatteryManager(int voltagePin, int pixelCount)
    : voltagePin(voltagePin), pixelCount(pixelCount) {}

float BatteryManager::getVoltage() {
    uint32_t Vbatt = 0;
    for (int i = 0; i < 16; i++) {
        Vbatt += analogReadMilliVolts(voltagePin); // ADC with correction   
    }
    return 2 * Vbatt / 16 / 1000.0; // attenuation ratio 1/2, mV --> V
}

float BatteryManager::convertToPercentage(float voltage) {
    const double constant = 123.0;
    const double divisor = 3.7;
    const double exponent1 = 80.0;
    const double exponent2 = 0.165;

    float result = constant - (constant / pow(1 + pow(voltage / divisor, exponent1), exponent2));
    return result;
}

int BatteryManager::convertToPixelCount(float percent) {
    float pixelCount = percent / (100 / (this->pixelCount - 1));
    int roundedCount = static_cast<int>(pixelCount + 0.5);
    roundedCount += 1;
    if (roundedCount < 1) {
        roundedCount = 1;
    }
    if (roundedCount > 16) {
        roundedCount = 16;
    }
    return roundedCount;
}

bool BatteryManager::checkBattery() {
    float voltage = getVoltage();
    int voltagePercent = convertToPercentage(voltage);

    if (voltagePercent < 5) {
        Serial.println("Battery empty!");
        esp_deep_sleep_start();
        return true;
    }
    return false;
}

int BatteryManager::getVoltageInPixels() {
    float voltage = getVoltage();
    Serial.printf("Voltage: %.2f\n", voltage);
    float voltagePercent = convertToPercentage(voltage);
    Serial.printf("Voltage percent: %.2f\n", voltagePercent);
    return convertToPixelCount(voltagePercent);
}