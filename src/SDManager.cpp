#include "SDManager.h"

SDManager::SDManager(uint8_t csPin) : csPin(csPin) {}

bool SDManager::begin() {
  return SD.begin(csPin);
}

std::vector<String> SDManager::readFileNames() {
    std::vector<String> names;
    File root = SD.open("/");

    while (true) {
    File entry = root.openNextFile();
    if (!entry) {
        // No more files
        break;
    }
    if (entry.isDirectory()) {
        // Skip directories
        continue;
    }

    String fileName = entry.name();
    if (fileName.endsWith(".mp3")) {
        Serial.println(fileName);
        names.push_back(fileName);
    }

    entry.close();
  }

  root.close();
  return names;
}

void SDManager::writeFile(const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}