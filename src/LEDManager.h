#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Adafruit_NeoPixel.h>

class LEDManager {
public:
  LEDManager(uint16_t pixelCount, uint8_t pixelPin, neoPixelType type = NEO_GBR + NEO_KHZ800);
  void begin();
  void show();
  void setBrightness(uint8_t brightness);
  void setPixelColor(uint16_t n, uint32_t color);
  void clear();
  void colorWipe(uint32_t color, int wait, int number = -1);
  void colorSet(uint32_t color, int number = -1);
  void rainbow(int wait);
  uint32_t Color(uint8_t r, uint8_t g, uint8_t b);

private:
  Adafruit_NeoPixel strip;
};

#endif // LED_MANAGER_H