#include "BatteryManager.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

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
    float minVoltage = 3.3;
    float maxVoltage = 4.20;
    float voltageRange = maxVoltage - minVoltage;
    return ((voltage - minVoltage) / voltageRange) * 100;
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
        mainLed.setBrightness(20);
        mainLed.colorWipe(mainLed.Color(255, 255, 255), 10, 1); // Red
        esp_deep_sleep_start();
        return true;
    }
    return false;
}

int BatteryManager::getVoltageInPixels() {
    float voltage = getVoltage();
    float voltagePercent = convertToPercentage(voltage);
    return convertToPixelCount(voltagePercent);
}