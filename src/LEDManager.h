#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <vector>

class LEDManager {
public:
    LEDManager(uint16_t pixelCount, uint8_t pixelPin, neoPixelType type = NEO_GRB + NEO_KHZ800);
    void begin();
    void show();
    void setBrightness(uint8_t brightness);
    void setPixelColor(uint16_t n, uint32_t color);
    void clear();
    void colorWipe(uint32_t color, int wait, int number = -1);
    void colorSet(uint32_t color, int number = -1, const String& energyMode = "full");
    void setColorFromString(const String& hexColor, const String& energyMode = "full");
    void setColorsEvenlySave(const std::vector<String>& colors);
    void setColorsEvenlyFull(const std::vector<String>& colors);
    void setColorsEvenly(const std::vector<String>& colors, const String& energyMode);
    void rainbow(int wait);
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b);

private:
    Adafruit_NeoPixel strip;
    uint32_t hexStringToColor(const String& hexColor);
    int skipPixelCount(const String& energyMode);
};

#endif // LEDMANAGER_H