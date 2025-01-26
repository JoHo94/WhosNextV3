#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>
#include <string>
#include <Arduino.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define JSON_CHARACTERISTIC_UUID "19B10002-E8F2-537E-4F6C-D104768A1215"
#define DATA_CHARACTERISTIC_UUID "19B10003-E8F2-537E-4F6C-D104768A1216"

class BluetoothManager {
private:
  NimBLEServer *pServer = nullptr;
  NimBLECharacteristic *jsonCharacteristic = nullptr;
  NimBLECharacteristic *dataCharacteristic = nullptr;
  bool deviceConnected = false;
  bool oldDeviceConnected = false;
  void (*connectionCallback)(bool connected) = nullptr;
  void (*configReceivedCallback)(String rawJson) = nullptr;

  class BluetoothCallbacks : public NimBLEServerCallbacks {
    BluetoothManager &parent;
  public:
    BluetoothCallbacks(BluetoothManager &parentRef) : parent(parentRef) {}
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo) override {
      parent.deviceConnected = true;
      if (parent.connectionCallback) {
        parent.connectionCallback(true);
      }
    }
    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo, int reason) override {
      Serial.println("Client disconnected - start advertising");
      NimBLEDevice::startAdvertising();
      parent.deviceConnected = false; 
      if (parent.connectionCallback) {
        parent.connectionCallback(false);
      }
    }
  };
  
  class JsonCallback : public NimBLECharacteristicCallbacks {
    BluetoothManager &parent;
  public:
    JsonCallback(BluetoothManager &parentRef) : parent(parentRef) {}
    String v = "";
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override {
      String value = pCharacteristic->getValue();
      v += value;
      if (value.endsWith("z")){
        // Drop last character
        v = v.substring(0, v.length()-1);
        if(parent.configReceivedCallback){
          parent.configReceivedCallback(v);
        }
        Serial.println(v);
        v = "";
      }
    }
  };

public:
  BluetoothManager() {}

  void setConnectionCallback(void (*callback)(bool connected)) {
    connectionCallback = callback;
  }
  
  void setConfigReceivedCallback(void (*callback)(String rawJson)) {
    configReceivedCallback = callback;
  }

  void init() {
    Serial.begin(115200);
    Serial.printf("Starting NimBLE Server\n");

    NimBLEDevice::init("ESP32");
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new BluetoothCallbacks(*this));
    NimBLEService*        pService = pServer->createService(SERVICE_UUID);
    jsonCharacteristic = pService->createCharacteristic( JSON_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY );
    dataCharacteristic = pService->createCharacteristic( DATA_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY );
    jsonCharacteristic->setCallbacks(new JsonCallback(*this));
    pService->start();
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("ESP32");
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    Serial.printf("Advertising Started\n");
  }

  void sendJson(const String& json) {

    if (!dataCharacteristic || !deviceConnected) {
      Serial.println("No device connected, cannot send JSON.");
      return;
    }
    Serial.println("Sending JSON");
    Serial.println(json.c_str());
    int length = json.length();
    int chunkSize = 200;
    for (int i = 0; i < length; i += chunkSize) {
      String chunk = json.substring(i, i + chunkSize);
      dataCharacteristic->setValue(chunk.c_str());
      dataCharacteristic->notify();
      delay(10); // Small delay to ensure the chunk is sent
    }
  }
};

#endif // BLUETOOTHMANAGER_H