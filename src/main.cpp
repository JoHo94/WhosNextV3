#include <Adafruit_NeoPixel.h>
#include "Arduino.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <OneButton.h>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "RemoteDebug.h"  //https://github.com/JoaoLopesF/RemoteDebug

#include "secrets.h"

// Digital I/O used
#define SD_CS         21 // 34 5
#define SPI_MOSI      D10 // 36 23
#define SPI_MISO      D9 // 38 19
#define SPI_SCK       D8 // 35 18
#define I2S_DOUT      A1 // 14
#define I2S_BCLK      A2 //2 4
#define I2S_LRC       A3 //15

#define PIXEL_PIN    A5  // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 16  // Number of NeoPixels

#define VOLTAGE_PIN A4

#define BUTTON_MAIN   D6
#define BUTTON_SEC_1 D7
#define Button_SEC_2 D0

RemoteDebug Debug;

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


Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GBR + NEO_KHZ800);
Audio audio;
//WiFiMulti wifiMulti;
//String ssid = "";
//String password = "";

int playOrder[20]; // Saves the current random playOrder
int currentSong = 101;
int fileCount = 0;
const int maxSize = 40;
String fileNames[maxSize];

int volume = 19;
int settingsMenu = 0; // 0 = no settings, 1 = Lautstärke, 2 = Spieleranzahl
int mode = 0;

void colorWipe(uint32_t color, int wait, int number = strip.numPixels()) {
  for(int i=0; i<number; i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void colorSet(uint32_t color, int number = strip.numPixels()) {
  strip.clear();
  for(int i=0; i<number; i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
  }
  strip.show();
}

void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

void listFilesInSDCard(String fileNames[], int maxSize) {
  File root = SD.open("/");
  fileCount = 0;

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
      Debug.println(fileName);
    fileNames[fileCount] = fileName;
    fileCount++;
    }

    entry.close();

    if (fileCount >= maxSize) {
      // Reached the maximum size of the array
      break;
    }
  }

  root.close();
}

void fillPlayOrderArray(int songCount) {
  for(int i = 0; i < songCount; i++){
    playOrder[i] = i;
  }

  for(int i = songCount-1; i > 1; i--){
    int j = random(i);
    int tmp = playOrder[i];
    playOrder[i] = playOrder[j];
    playOrder[j] = tmp;
  }

  Debug.println("Playorder:");
  for(int i = 0; i < songCount; i++){
    Debug.println(playOrder[i]);
  }
}

void playSong(int index, int orderIndex = 0) {
      Debug.print("Play Index ");
      Debug.print(orderIndex);
      Debug.print("-> ");
      Debug.print(index);
      Debug.print(" = ");
      Debug.println(fileNames[index].c_str());
      audio.connecttoSD(fileNames[index].c_str());
 }


float convertToPercentage(float voltage) {
  float minVoltage = 3.3;
  float maxVoltage = 4.20;
  float voltageRange = maxVoltage - minVoltage;
  float percentage = ((voltage - minVoltage) / voltageRange) * 100;
  return percentage;
}

int convertToPixelCount(float percent){
  float pixelCount = percent / (100/(PIXEL_COUNT-1));
  int roundedCount = (int)pixelCount+.5;
  roundedCount += 1;
  if(roundedCount < 1){
    roundedCount = 1;
  }
  if(roundedCount > 16){
    roundedCount = 16;
  }
  return roundedCount;
}

float getVoltage(){
  uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(VOLTAGE_PIN); // ADC with correction   
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0;     // attenuation ratio 1/2, mV --> V
  return Vbattf;
}

void applyNewVolume(){
    audio.setVolume(volume);
    int ledCount = volume - 5;
    colorSet(strip.Color(  255,   255, 255), ledCount);
    Debug.print("Changed volume to: ");
    Debug.println(volume);
}

// Handler function for a single click:
static void handleSec1Click() {
  Debug.println("Left Button Clicked!");
  if (settingsMenu == 1){
    if (volume - 2 >= 5) { //Never smaller than 5
        volume -= 2;
    } else {
        volume = 5; // Ensure it doesn't go below 0
    }
    applyNewVolume();
    audio.connecttoSD("/Settings/Leiser.mp3");
  }
}

// Handler function for a single click:
static void handleSec2Click() {
  Debug.println("Right Button Clicked!");
  if (settingsMenu == 1){
    if (volume + 2 <= 21) { //Not greater than 21
      volume += 2;
    } else {
      volume = 21; // Ensure it doesn't exceed 21
    }
    applyNewVolume();
    audio.connecttoSD("/Settings/Lauter.mp3");
  }
}

// Handler function for a single click:
static void handleMainButtonLongClick() {
  Debug.println("Main Longpress");
  if(settingsMenu == 0){
    settingsMenu = 1;
    audio.connecttoSD("/Settings/Lautstärke.mp3");
    applyNewVolume();
    return;
  }
  if (settingsMenu == 1){
    settingsMenu = 0;
    rainbow(0);
    return;
  }
  
} 

// Handler function for a single click:
static void handleMainButtonClick() {
  Debug.println("MainButton Clicked!!");
  if(currentSong < fileCount){
    playSong(playOrder[currentSong], currentSong);
    currentSong++;
  } else {
    fillPlayOrderArray(fileCount);
    currentSong = 1;
    playSong(playOrder[0], 0);
  }

  if(++mode > 3) mode = 0; // Advance to next mode, wrap around after #8
  switch(mode) {           // Start the new animation...
    case 0:
      colorWipe(strip.Color(  0,   165,   255, 10), 10);    // Orange
      break;
    case 1:
      colorWipe(strip.Color(  0, 255,   0, 64), 10);    // Green
      break;
    case 2:
      colorWipe(strip.Color(255,   0,   0, 64), 10);    // Red
      break;
    case 3:
      colorWipe(strip.Color(  0,   0, 255, 64), 10);    // Blue
      break;
  }
}

void initWifiAndOta()
 {
  Debug.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
     //while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //   Debug.println("Connection Failed! Rebooting...");
     //  delay(5000);
    //   ESP.restart();
     //}
  
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.setHostname("thebutton");
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Debug.println("Start updating " + type);
    })
    .onEnd([]() {
      Debug.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Debug.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Debug.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Debug.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Debug.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Debug.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Debug.println("Receive Failed");
      else if (error == OTA_END_ERROR) Debug.println("End Failed");
    });

    ArduinoOTA.begin();

  Debug.println("Ready");
  Debug.print("IP address: ");
  Debug.println(WiFi.localIP());
}

void setup() {
  
  initWifiAndOta();

  Debug.begin("thebutton");
  Debug.setResetCmdEnabled(true); // Enable the reset command
  pinMode(VOLTAGE_PIN, INPUT);
  int voltagePixelCount = convertToPixelCount(convertToPercentage(getVoltage()));

  strip.begin();
  strip.show();
  strip.setBrightness(50);
  colorWipe(strip.Color(  0, 255,   0, 64), 10, voltagePixelCount);    // Green

  Serial.begin(115200);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  SD.begin(SD_CS);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume); // 0...21

  btnSec1.attachClick(handleSec1Click);
  btnSec2.attachClick(handleSec2Click);
  btnMain.attachClick(handleMainButtonClick);
  btnMain.attachLongPressStart(handleMainButtonLongClick);

  listFilesInSDCard(fileNames, maxSize);

  // Print the file names
  for (int i = 0; i < maxSize; i++) {
    if (fileNames[i] != "") {
      Debug.println(fileNames[i]);
    } else {
      break;
    }
  }
  Debug.print("Filecount:");
  Debug.println(fileCount);
  
  fillPlayOrderArray(fileCount);

}


void loop()
{
  float voltage = getVoltage();
  int voltagePercent = convertToPercentage(voltage);
  //Debug.print("Voltage: ");
  //Debug.print(voltage);
  //Debug.print(" -> ");
  //Debug.print(voltagePercent);
  //Debug.print(" -> ");
  //Debug.println(convertToPixelCount(voltagePercent));

  if(voltagePercent < 5){
    Debug.println("Battery empty!");
    strip.setBrightness(20);
    colorWipe(strip.Color(255,   255,   255, 255), 10, 1);    // Red
    esp_deep_sleep_start();
    return;
  }
  audio.loop();
  btnSec1.tick();
  btnSec2.tick();
  btnMain.tick();
  
  ArduinoOTA.handle();
  Debug.handle();
}

