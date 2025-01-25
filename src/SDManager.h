#ifndef SDMANAGER_H
#define SDMANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <vector>

class SDManager {
public:
    SDManager(int csPin);
    void writeFile(const char* path, const char* message);
    String readFile(const char* path);
    std::vector<String> readFileNames();
    std::vector<String> listFiles(const char* dirname, uint8_t levels);

private:
    int csPin;
};

#endif // SDMANAGER_H