#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class LEDManager {
public:
    LEDManager(uint16_t pixelCount, uint8_t pixelPin, neoPixelType type = NEO_GRB + NEO_KHZ800);
    void begin();
    void show();
    void setBrightness(uint8_t brightness);
    void setPixelColor(uint16_t n, uint32_t color);
    void clear();
    void colorWipe(uint32_t color, int wait, int number = -1);
    void colorSet(uint32_t color, int number = -1);
    void setColorFromString(const String& hexColor);
    void rainbow(int wait);
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
    void setColorsEvenly(const std::vector<String>& colors);

private:
    Adafruit_NeoPixel strip;
    uint32_t hexStringToColor(const String& hexColor);
};

#endif // LEDMANAGER_H