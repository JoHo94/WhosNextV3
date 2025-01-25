#include "ConfigManager.h"
#include <Adafruit_NeoPixel.h>

ConfigManager::ConfigManager(SDManager& sdManager) : doc(1024), sdManager(sdManager) { }
void ConfigManager::saveConfig(const String& rawJson) {
    Serial.print("Received config: ");
    Serial.println(rawJson);

    deserializeJson(doc, rawJson);
    String jsonString;
    serializeJson(doc, jsonString);
    sdManager.writeFile("/config.txt", jsonString.c_str());
    currentConfig.playerColors.clear();
    for (JsonVariant v : doc["playerOrder"].as<JsonArray>()) {
        currentConfig.playerColors.push_back(v.as<String>());
    }
    currentConfig.brightness = doc["brightness"];
    currentConfig.volume = doc["volume"];
    currentConfig.energyMode = doc["energyMode"].as<String>();

    if (configCallback) {
        configCallback(currentConfig);
    }
}

void ConfigManager::initConfig() {
    String jsonString = sdManager.readFile("/config.txt");
    if (!jsonString.isEmpty()) {
        deserializeJson(doc, jsonString);
        currentConfig.playerColors.clear();
        for (JsonVariant v : doc["playerOrder"].as<JsonArray>()) {
            currentConfig.playerColors.push_back(v.as<String>());
        }
        currentConfig.brightness = doc["brightness"];
        currentConfig.volume = doc["volume"];
        currentConfig.energyMode = doc["energyMode"].as<String>();

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