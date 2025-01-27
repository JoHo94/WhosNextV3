#include <Adafruit_NeoPixel.h>
#include "Arduino.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <OneButton.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <NimBLEDevice.h>

#include "secrets.h"

#include "BluetoothManager.h"
#include "LEDManager.h"
#include "SDManager.h"
#include "pinout.h"
#include "PlayOrderManager.h"
#include "BatteryManager.h"
#include "ConfigManager.h"

//Pin, activeLow, pullupActive
OneButton btnSec1 = OneButton( BUTTON_SEC_1, true, true);
OneButton btnSec2 = OneButton( Button_SEC_2, true, true );
OneButton btnMain = OneButton( BUTTON_MAIN, true, true );

LEDManager mainLed(PIXEL_COUNT, PIXEL_PIN);
LEDManager statusLed(1, STATUS_PIN);

BluetoothManager bluetoothManager;
SDManager sdManager(SD_CS);
ConfigManager configManager(sdManager);
Audio audio;
PlayOrderManager* playOrderManager = nullptr;
BatteryManager batteryManager(VOLTAGE_PIN, PIXEL_COUNT);

int currentPlayer = 0;

bool configChanged = false;
Config newConfig;

bool freshConnected = false;

unsigned long advertisingStartTime = -1;
const unsigned long advertisingDuration = 30000; // 30 seconds

void startAdvertising() {
    Serial.println("Starting Bluetooth advertising...");
    bluetoothManager.startAdvertising();
    advertisingStartTime = millis();
    statusLed.setPixelColor(0, statusLed.Color(0,0,255));
}

void stopAdvertising() {
    if (!bluetoothManager.isDeviceConnected()) {
        Serial.println("Stopping Bluetooth advertising...");
        bluetoothManager.stopAdvertising();
        statusLed.clear();
    }
}

void playSong(const String& filename) { 
        audio.connecttoFS(SD, filename.c_str());
} 

void applyConfig(const Config& config) {
    Serial.print("Config changed to: ");
    Serial.println(config.toString());

    newConfig = config;
    configChanged = true;
}

void applyConfigInLoop() {
    if (!newConfig.playerColors.empty()) {
       mainLed.setColorsEvenly(newConfig.playerColors, newConfig.energyMode);
    }else{
        mainLed.clear();
    }
    audio.setVolume(newConfig.volume);
    mainLed.setBrightness(newConfig.brightness);
}

void applyNewVolume(int newVolume){
    audio.setVolume(newVolume);
    Serial.print("Changed volume to: ");
    Serial.println(newVolume);
    newConfig.volume = newVolume;
    configManager.saveConfig(newConfig.toJson());
    bluetoothManager.sendJson(newConfig.toJson());
}

// Handler function for a single click:
static void handleSec1Click() {
  Serial.println("Left Button Clicked!");
  int volume = newConfig.volume;
  if (volume - 2 >= 4) { //Never smaller than 5
      volume -= 2;
  } else {
      volume = 4; // Ensure it doesn't go below 0
  }
  applyNewVolume(volume);
  playSong("/Settings/Leiser.mp3");
}

// Handler function for a single click:
static void handleSec2Click() {
  Serial.println("Right Button Clicked!");
  int volume = newConfig.volume;
  if (volume + 2 <= 21) { //Not greater than 21
    volume += 2;
  } else {
    volume = 21; // Ensure it doesn't exceed 21
  }
  applyNewVolume(volume);
  playSong("/Settings/Lauter.mp3");
}

static void handleSec2LongClickStart() {
  Serial.println("Right Button Long Clicked Start!");
  mainLed.colorSet(mainLed.Color(0,255,0), batteryManager.getVoltageInPixels(), "full");
  //playSong("/Settings/Lauter.mp3"); Battery sound here!
}

// Handler function for a single click:
static void handleSec1LongClickStart() {
  Serial.println("Left Button Long Clicked Start!");

  const int brightnessLevels[] = {32, 64, 128, 192, 255};
  const int numLevels = sizeof(brightnessLevels) / sizeof(brightnessLevels[0]);

  for (int i = 0; i < numLevels; ++i) {
    if (newConfig.brightness < brightnessLevels[i]) {
      newConfig.brightness = brightnessLevels[i];
      break;
    } else if (newConfig.brightness == brightnessLevels[numLevels - 1]) {
      newConfig.brightness = brightnessLevels[0];
      break;
    }
  }
  configManager.saveConfig(newConfig.toJson());
  bluetoothManager.sendJson(newConfig.toJson());
  mainLed.setBrightness(newConfig.brightness);

  //playSong("/Settings/Lauter.mp3"); Battery sound here!
}

// Handler function for a single click:
static void handleSec2LongClickStop() {
  Serial.println("Right Button Long Clicked End!");

  String playerColor = newConfig.playerColors[currentPlayer];
  mainLed.setColorFromString(playerColor, newConfig.energyMode);
  //playSong("/Settings/Lauter.mp3"); Battery sound here!
}

// Handler function for a single click:
static void handleMainButtonLongClick() {
  Serial.println("Main Longpress");
  startAdvertising();
} 

void setColorForNextPlayer() {
    currentPlayer++;
    if (currentPlayer >= newConfig.playerColors.size()) {
        currentPlayer = 0;
    }
    String playerColor = newConfig.playerColors[currentPlayer];
    mainLed.setColorFromString(playerColor, newConfig.energyMode);
}

// Handler function for a single click:
static void handleMainButtonClick() {
  Serial.println("MainButton Clicked!!");
  playSong(playOrderManager->getCurrentSong());
  playOrderManager->nextSong();
  setColorForNextPlayer();
}


void onBluetoothConnection(bool connected) {
  if (connected) {
    Serial.println("Device connected!");
    statusLed.setPixelColor(0, statusLed.Color(0,255,0));
    freshConnected = true;
  } else {
    Serial.println("Device disconnected!");
    statusLed.setPixelColor(0, statusLed.Color(0,0,255));
    startAdvertising(); // Restart advertising when the device disconnects
  }
}

void configReceived(String rawJson) {
  configManager.saveConfig(rawJson);
}

void setup() {
    
  Serial.begin(115200);
  bluetoothManager.init();
  bluetoothManager.setConnectionCallback(onBluetoothConnection); // Register the callback
  bluetoothManager.setConfigReceivedCallback(configReceived); // Register the callback
  
  configManager.setConfigCallback(applyConfig);
  configManager.initConfig();

  pinMode(VOLTAGE_PIN, INPUT);

  statusLed.colorSet(statusLed.Color(0, 0, 255));

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  SD.begin(SD_CS);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(newConfig.volume); // 0...21

  btnSec1.attachClick(handleSec1Click);
  btnSec2.attachClick(handleSec2Click);
  btnMain.attachClick(handleMainButtonClick);
  btnMain.attachLongPressStart(handleMainButtonLongClick);
  btnSec2.attachLongPressStart(handleSec2LongClickStart);
  btnSec2.attachLongPressStop(handleSec2LongClickStop);
  btnSec1.attachLongPressStart(handleSec1LongClickStart);

  std::vector<String> fileNames = sdManager.readFileNames();
  playOrderManager = new PlayOrderManager(fileNames);
  for (int i = 0; i < fileNames.size(); i++) {
    Serial.println(fileNames[i]);
  }

  startAdvertising(); // Start advertising on boot
}

void sendConfig() {
  delay(1000);
  bluetoothManager.sendJson(newConfig.toJson());
}

void loop()
{
  // float voltage = getVoltage();
  // int voltagePercent = convertToPercentage(voltage);
  //Serial.print("Voltage: ");
  //Serial.print(voltage);
  //Serial.print(" -> ");
  //Serial.print(voltagePercent);
  //Serial.print(" -> ");
  //Serial.println(convertToPixelCount(voltagePercent));
  //bluetoothManager.update(voltage);
  if (batteryManager.checkBattery()) {
      mainLed.setBrightness(0);
      statusLed.setPixelColor(0, statusLed.Color(255,0,0));
      return; // Break the loop if the battery is low
  }

  if (configChanged) {
      applyConfigInLoop();
      configChanged = false;
  }  
  if (freshConnected) {
      sendConfig();
      freshConnected = false;
  }
  // Stop advertising after 1 minute if no device is connected
  if (!bluetoothManager.isDeviceConnected() && millis() - advertisingStartTime >= advertisingDuration && advertisingStartTime != -1) {
      stopAdvertising();
      advertisingStartTime = -1;
  }

  audio.loop();
  btnSec1.tick();
  btnSec2.tick();
  btnMain.tick();
}

