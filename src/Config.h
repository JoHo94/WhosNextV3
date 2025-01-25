#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <Arduino.h>

struct Config {
    std::vector<String> playerColors = {"#FF0000", "#00FF00", "#0000FF", "#FFFF00"};
    int brightness = 50;
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
};

#endif // CONFIG_H