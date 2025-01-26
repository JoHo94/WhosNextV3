#include "ConfigManager.h"
#include <Adafruit_NeoPixel.h>

ConfigManager::ConfigManager(SDManager& sdManager) : doc(), sdManager(sdManager) { }
void ConfigManager::saveConfig(const String& rawJson) {
    Serial.print("Received config: ");
    Serial.println(rawJson);

    Config newConfig = Config().fromJson(rawJson);
    String validatesJson = newConfig.toJson();
    sdManager.writeFile("/config.txt", validatesJson.c_str());
    currentConfig = newConfig;
    if (configCallback) {
        configCallback(currentConfig);
    }
}

void ConfigManager::initConfig() {
    String jsonString = sdManager.readFile("/config.txt");
    if (!jsonString.isEmpty()) {
        Config newConfig = Config().fromJson(jsonString);
        currentConfig = newConfig;
        if (configCallback) {
            configCallback(currentConfig);
        }
    }
}

Config ConfigManager::getCurrentConfig() const {
    return currentConfig;
}

void ConfigManager::setConfigCallback(ConfigCallback callback) {
    configCallback = callback;
}