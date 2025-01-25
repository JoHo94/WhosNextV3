#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"

class SDManager {
public:
  int fileCount = 0;
  const int maxSize = 40;
  SDManager(uint8_t csPin);
  bool begin();
  std::vector<String> readFileNames();
  void writeFile(const char *path, const char *message);

private:
  uint8_t csPin;
};

#endif // SD_MANAGER_H