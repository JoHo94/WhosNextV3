#ifndef BATTERYMANAGER_H
#define BATTERYMANAGER_H

class BatteryManager {
public:
    BatteryManager(int voltagePin, int pixelCount);
    float getVoltage();
    bool checkBattery();
    int getVoltageInPixels();

private:
    float convertToPercentage(float voltage);
    int convertToPixelCount(float percent);

    int voltagePin;
    int pixelCount;
};

#endif // BATTERYMANAGER_H