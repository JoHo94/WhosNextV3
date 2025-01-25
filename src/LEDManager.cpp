#include "LEDManager.h"

LEDManager::LEDManager(uint16_t pixelCount, uint8_t pixelPin, neoPixelType type)
  : strip(pixelCount, pixelPin, type) {
    strip.begin();
  }

void LEDManager::begin() {
  strip.begin();
}

void LEDManager::show() {
  strip.show();
}

void LEDManager::setBrightness(uint8_t brightness) {
  strip.setBrightness(brightness);
  strip.show();
}

void LEDManager::setPixelColor(uint16_t n, uint32_t color) {
  strip.setPixelColor(n, color);
  strip.show();
}

void LEDManager::clear() {
  strip.clear();
  strip.show();
}

void LEDManager::colorWipe(uint32_t color, int wait, int number) {
  if (number == -1) number = strip.numPixels();
  strip.clear();
  for (int i = 0; i < number; i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

void LEDManager::colorSet(uint32_t color, int number) {
  if (number == -1) number = strip.numPixels();
  strip.clear();
  for (int i = 0; i < number; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void LEDManager::setColorFromString(const String& hexColor) {
  uint32_t color = hexStringToColor(hexColor);
  colorSet(color);
}

uint32_t LEDManager::Color(uint8_t r, uint8_t g, uint8_t b) {
  return strip.Color(r, g, b);
}

void LEDManager::rainbow(int wait) {
  for (long firstPixelHue = 0; firstPixelHue < 3 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < strip.numPixels(); i++) {
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(wait);
  }
}

void LEDManager::setColorsEvenly(const std::vector<String>& colors) {
  int numPixels = strip.numPixels();
  int numColors = colors.size();
  int segmentLength = numPixels / numColors;

  for (int i = 0; i < numColors; i++) {
    uint32_t color = hexStringToColor(colors[i]);
    for (int j = 0; j < segmentLength; j++) {
      strip.setPixelColor(i * segmentLength + j, color);
    }
  }

  // Handle any remaining pixels if the number of pixels is not evenly divisible by the number of colors
  for (int i = numColors * segmentLength; i < numPixels; i++) {
    strip.setPixelColor(i, hexStringToColor(colors[numColors - 1]));
  }

  strip.show();
}

uint32_t LEDManager::hexStringToColor(const String& hexColor) {
  int r = (int)strtol(hexColor.substring(1, 3).c_str(), nullptr, 16);
  int g = (int)strtol(hexColor.substring(3, 5).c_str(), nullptr, 16);
  int b = (int)strtol(hexColor.substring(5, 7).c_str(), nullptr, 16);
  return strip.Color(r, g, b);
}