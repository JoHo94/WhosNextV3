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

int volume = 21;
int settingsMenu = 0; // 0 = no settings, 1 = Lautstärke, 2 = Spieleranzahl
int currentPlayer = 0;
bool configChanged = false;
Config newConfig;

void playSong(const String& filename) {
    Serial.print("Playing: ");
    Serial.println(filename);
    audio.connecttoFS(SD, filename.c_str());
} 

void applyConfig(const Config& config) {
    Serial.print("Config changed to: ");
    Serial.println(config.toString());

    newConfig = config;
    configChanged = true;
    mainLed.setBrightness(newConfig.brightness);
}

void applyConfigInLoop() {
    if (!newConfig.playerColors.empty()) {
      mainLed.setColorsEvenly(newConfig.playerColors);
    }
    volume = newConfig.volume;
    audio.setVolume(volume);
    }

void applyNewVolume(){
    audio.setVolume(volume);
    int ledCount = volume - 5;
    mainLed.colorSet(mainLed.Color(255, 255, 255), ledCount);
    Serial.print("Changed volume to: ");
    Serial.println(volume);
}

// Handler function for a single click:
static void handleSec1Click() {
  Serial.println("Left Button Clicked!");
  if (settingsMenu == 1){
    if (volume - 2 >= 5) { //Never smaller than 5
        volume -= 2;
    } else {
        volume = 5; // Ensure it doesn't go below 0
    }
    applyNewVolume();
    playSong("/Settings/Leiser.mp3");
  }
}

// Handler function for a single click:
static void handleSec2Click() {
  Serial.println("Right Button Clicked!");
  if (settingsMenu == 1){
    if (volume + 2 <= 21) { //Not greater than 21
      volume += 2;
    } else {
      volume = 21; // Ensure it doesn't exceed 21
    }
    applyNewVolume();
    playSong("/Settings/Lauter.mp3");
  }
}

// Handler function for a single click:
static void handleMainButtonLongClick() {
  Serial.println("Main Longpress");
  if(settingsMenu == 0){
    settingsMenu = 1;
    playSong("/Settings/Lautstärke.mp3");
    applyNewVolume();
    return;
  }
  if (settingsMenu == 1){
    settingsMenu = 0;
    mainLed.rainbow(0);
    return;
  }
} 

void setColorForNextPlayer() {
    currentPlayer++;
    if (currentPlayer >= newConfig.playerColors.size()) {
        currentPlayer = 0;
    }
    String playerColor = newConfig.playerColors[currentPlayer];
    mainLed.setColorFromString(playerColor);
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
    statusLed.setPixelColor(0, mainLed.Color(0,255,0));
  } else {
    Serial.println("Device disconnected!");
    statusLed.Color(255, 0, 0);
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

  // strip.begin();
  // strip.show();
  // mainLed.setBrightness(50);
  // colorWipe(strip.Color(  0, 255,   0), 10, voltagePixelCount);    // Green

  // statusLed.begin();
  // statusLed.setBrightness(50);
  // colorWipe(statusLed.Color(  255, 0,   0), 10, voltagePixelCount);    // Green
  // statusLed.show();

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  SD.begin(SD_CS);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume); // 0...21

  btnSec1.attachClick(handleSec1Click);
  btnSec2.attachClick(handleSec2Click);
  btnMain.attachClick(handleMainButtonClick);
  btnMain.attachLongPressStart(handleMainButtonLongClick);

  // read files to vector
  std::vector<String> fileNames = sdManager.readFileNames();
  playOrderManager = new PlayOrderManager(fileNames);
  // print file names
  for (int i = 0; i < fileNames.size(); i++) {
    Serial.println(fileNames[i]);
  }
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
      mainLed.setBrightness(20);
      mainLed.colorWipe(mainLed.Color(255, 255, 255), 10, 1); // Red
      return; // Break the loop if the battery is low
  }

  if (configChanged) {
      applyConfigInLoop();
      configChanged = false;
  }

  audio.loop();
  btnSec1.tick();
  btnSec2.tick();
  btnMain.tick();
}

