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

OneButton btnSec1 = OneButton(
  BUTTON_SEC_1,  // Input pin for the button
  true,        // Button is active LOW
  true         // Enable internal pull-up resistor
);

OneButton btnSec2 = OneButton(
  Button_SEC_2,  // Input pin for the button
  true,        // Button is active LOW
  true         // Enable internal pull-up resistor
);

OneButton btnMain = OneButton(
  BUTTON_MAIN,  // Input pin for the button
  true,        // Button is active LOW
  true         // Enable internal pull-up resistor
);

LEDManager mainLed(PIXEL_COUNT, PIXEL_PIN);
LEDManager statusLed(1, STATUS_PIN);

BluetoothManager bluetoothManager;
SDManager sdManager(SD_CS); // Initialize SDManager
Audio audio;
PlayOrderManager* playOrderManager = nullptr;
BatteryManager batteryManager(VOLTAGE_PIN, PIXEL_COUNT);

int playOrder[20]; // Saves the current random playOrder
int currentSong = 101;

int volume = 21;
int settingsMenu = 0; // 0 = no settings, 1 = Lautstärke, 2 = Spieleranzahl
int mode = 0;

void playSong(const String& filename) {
    Serial.print("Playing: ");
    Serial.println(filename);
    audio.connecttoFS(SD, filename.c_str());
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
    audio.connecttoFS(SD,"/Settings/Leiser.mp3");
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
    audio.connecttoFS(SD,"/Settings/Lauter.mp3");
  }
}

// Handler function for a single click:
static void handleMainButtonLongClick() {
  Serial.println("Main Longpress");
  if(settingsMenu == 0){
    settingsMenu = 1;
    audio.connecttoFS(SD,"/Settings/Lautstärke.mp3");
    applyNewVolume();
    return;
  }
  if (settingsMenu == 1){
    settingsMenu = 0;
    mainLed.rainbow(0);
    return;
  }
} 

// Handler function for a single click:
static void handleMainButtonClick() {
  Serial.println("MainButton Clicked!!");
        playSong(playOrderManager->getCurrentSong());
        playOrderManager->nextSong();


      if(++mode > 3) mode = 0; // Advance to next mode, wrap around after #8
      switch(mode) {           // Start the new animation...
        case 0:
          mainLed.colorWipe(mainLed.Color(  0,   165,   255), 10);    // Orange
          break;
        case 1:
          mainLed.colorWipe(mainLed.Color(  0, 255,   0), 10);    // Green
          break;
        case 2:
          mainLed.colorWipe(mainLed.Color(255,   0,   0), 10);    // Red
          break;
        case 3:
          mainLed.colorWipe(mainLed.Color(  0,   0, 255), 10);    // Blue
          break;
      }
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
  Serial.println("Received config:");
  Serial.println(rawJson);
  JsonDocument doc;
  deserializeJson(doc, rawJson);
  
  sdManager.writeFile("/config.txt", rawJson.c_str());

  String playerColor1 = doc["playerOrder"][0];
  int brightness = doc["brightness"];
  int volume = doc["volume"];
  String energyMode = doc["energyMode"];

  Serial.print("Player color 1: ");
  Serial.println(playerColor1);
  Serial.print("Brightness: ");
  Serial.println(brightness);
  Serial.print("Volume: ");
  Serial.println(volume);
  Serial.print("Energy mode: ");
  Serial.println(energyMode);

  int r = (int)strtol(playerColor1.substring(1, 3).c_str(), nullptr, 16);
  int g = (int)strtol(playerColor1.substring(3, 5).c_str(), nullptr, 16);
  int b = (int)strtol(playerColor1.substring(5, 7).c_str(), nullptr, 16);

  mainLed.colorSet(mainLed.Color(r, g, b));
}

void setup() {
    
  Serial.begin(115200);
  bluetoothManager.init();
  bluetoothManager.setConnectionCallback(onBluetoothConnection); // Register the callback
  bluetoothManager.setConfigReceivedCallback(configReceived); // Register the callback

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
      return; // Break the loop if the battery is low
  }
  audio.loop();
  btnSec1.tick();
  btnSec2.tick();
  btnMain.tick();
}

