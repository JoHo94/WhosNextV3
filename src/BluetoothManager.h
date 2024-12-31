#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <NimBLEDevice.h>
//#include <BLEServer.h>
//#include <BLEUtils.h>
//#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>
#include <string>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define COLOR_CHARACTERISTIC_UUID "19B10002-E8F2-537E-4F6C-D104768A1214"

class BluetoothManager {
private:
  NimBLEServer *pServer = nullptr;
  NimBLECharacteristic *pCharacteristic = nullptr;
  NimBLECharacteristic *pColorCharacteristic = nullptr;
  Adafruit_NeoPixel &strip;
  bool deviceConnected = false;
  bool oldDeviceConnected = false;
  void (*connectionCallback)(bool connected) = nullptr;

  class MyServerCallbacks : public NimBLEServerCallbacks {
    BluetoothManager &parent;
  public:
    MyServerCallbacks(BluetoothManager &parentRef) : parent(parentRef) {}
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo) override {
      parent.deviceConnected = true;
      if (parent.connectionCallback) {
        parent.connectionCallback(true);
      }
    }
    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo, int reason) override {
      parent.deviceConnected = false;
      if (parent.connectionCallback) {
        parent.connectionCallback(false);
      }
    }
  };

  class ColorCallback : public NimBLECharacteristicCallbacks {
    Adafruit_NeoPixel &strip;
  public:
    ColorCallback(Adafruit_NeoPixel &stripRef) : strip(stripRef) {}
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override {
      String value = pCharacteristic->getValue();
      Serial.println(value);
      int r = (int)strtol(value.substring(1, 3).c_str(), nullptr, 16);
      int g = (int)strtol(value.substring(3, 5).c_str(), nullptr, 16);
      int b = (int)strtol(value.substring(5, 7).c_str(), nullptr, 16);

        strip.clear();
        for(int i=0; i<16; i++) { // For each pixel in strip...
            strip.setPixelColor(i, strip.Color(r,g,b));         //  Set pixel's color (in RAM)
        }
        strip.show();                          //  Update strip to match
    }
  };

public:
  BluetoothManager(Adafruit_NeoPixel &stripRef) : strip(stripRef) {}

  void setConnectionCallback(void (*callback)(bool connected)) {
    connectionCallback = callback;
  }

  void init() {
    Serial.begin(115200);
    Serial.printf("Starting NimBLE Server\n");

    NimBLEDevice::init("ESP32");
    pServer = NimBLEDevice::createServer();
    NimBLEService*        pService = pServer->createService(SERVICE_UUID);
    NimBLECharacteristic* pCharacteristic = pService->createCharacteristic( CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY| NIMBLE_PROPERTY::INDICATE );
    NimBLECharacteristic* pColorCharacteristic = pService->createCharacteristic( COLOR_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE );
    pColorCharacteristic->setCallbacks(new ColorCallback(strip));
    pService->start();
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("ESP32");
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    Serial.printf("Advertising Started\n");
  }

  void update(float voltage) {
    if (deviceConnected) {
      pCharacteristic->setValue(String(voltage));
      Serial.println(voltage);
      pCharacteristic->notify();
      delay(500);
    }

    if (!deviceConnected && oldDeviceConnected) {
      delay(500);
      pServer->startAdvertising();
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
    }
  }
};

#endif // BLUETOOTH_MANAGER_H