#ifndef PLAYORDERMANAGER_H
#define PLAYORDERMANAGER_H

#include <vector>
#include <Arduino.h>

class PlayOrderManager {
public:
    PlayOrderManager(const std::vector<String>& filenames);
    ~PlayOrderManager();

    String getCurrentSong();
    void nextSong();
    void resetPlayOrder();

private:
    void fillPlayOrderArray();

    std::vector<String> filenames;
    int* playOrder;
    int maxSongs;
    int currentSong;
};

#endif // PLAYORDERMANAGER_H