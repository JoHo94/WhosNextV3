#include "SDManager.h"

SDManager::SDManager(int csPin) : csPin(csPin) {
    if (!SD.begin(csPin)) {
        Serial.println("Card Mount Failed");
        return;
    }
}

void SDManager::writeFile(const char* path, const char* message) {
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

String SDManager::readFile(const char* path) {
    File file = SD.open(path);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return "";
    }

    String content;
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();
    return content;
}

std::vector<String> SDManager::listFiles(const char* dirname, uint8_t levels) {
    std::vector<String> fileNames;
    File root = SD.open(dirname);

    if (!root) {
        Serial.println("Failed to open directory");
        return fileNames;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return fileNames;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            if (levels) {
                std::vector<String> subDirFiles = listFiles(file.name(), levels - 1);
                fileNames.insert(fileNames.end(), subDirFiles.begin(), subDirFiles.end());
            }
        } else {
            fileNames.push_back(String(file.name()));
        }
        file = root.openNextFile();
    }
    return fileNames;
}