#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <ArduinoJson.h>
#include <vector>
#include <Arduino.h>
#include "Config.h"
#include "SDManager.h"
#include <functional>

class ConfigManager {
public:
    using ConfigCallback = std::function<void(const Config&)>;

    ConfigManager(SDManager& sdManager);
    void saveConfig(const String& rawJson);
    Config getCurrentConfig() const;
    void setConfigCallback(ConfigCallback callback);
    void initConfig();

private:
    DynamicJsonDocument doc;
    Config currentConfig;
    SDManager& sdManager;
    ConfigCallback configCallback;

};

#endif // CONFIGMANAGER_H