#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <Arduino.h>

struct Config {
    std::vector<String> playerColors = {"#FF0000", "#00FF00", "#0000FF", "#FFFF00"};
    int brightness = 128;
    int volume = 21;
    String energyMode = "normal";

    String toString() const {
        String result = "Config: ";
        result += "Player Colors: ";
        for (const auto& color : playerColors) {
            result += color + " ";
        }
        result += " Brightness: " + String(brightness) + " ";
        result += "Volume: " + String(volume) + " ";
        result += "Energy Mode: " + energyMode + "\n";
        return result;
    }

    String toJson() const {
        // with arduino json library
        JsonDocument doc;
        JsonArray colorsArray = doc["playerOrder"].to<JsonArray>();
        for (const auto& color : playerColors) {
            colorsArray.add(color);
        }
        doc["brightness"] = brightness;
        doc["volume"] = volume;
        doc["energyMode"] = energyMode;
        String result;
        serializeJson(doc, result);
        return result;
    }

    Config fromJson(const String& json) {
        JsonDocument doc;
        deserializeJson(doc, json);
        playerColors.clear();
        for (JsonVariant v : doc["playerOrder"].as<JsonArray>()) {
            playerColors.push_back(v.as<String>());
        }
        brightness = doc["brightness"];
        volume = doc["volume"];
        energyMode = doc["energyMode"].as<String>();
        return *this;
    }
}; 

#endif // CONFIG_H