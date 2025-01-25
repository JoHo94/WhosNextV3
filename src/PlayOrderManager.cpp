#include "PlayOrderManager.h"
#include <Arduino.h>

PlayOrderManager::PlayOrderManager(const std::vector<String>& filenames) 
    : filenames(filenames), maxSongs(filenames.size()), currentSong(0) {
    playOrder = new int[maxSongs];
    fillPlayOrderArray();
}

PlayOrderManager::~PlayOrderManager() {
    delete[] playOrder;
}

void PlayOrderManager::fillPlayOrderArray() {
    for (int i = 0; i < maxSongs; i++) {
        playOrder[i] = i;
    }

    for (int i = maxSongs - 1; i > 1; i--) {
        int j = random(i);
        int tmp = playOrder[i];
        playOrder[i] = playOrder[j];
        playOrder[j] = tmp;
    }

    Serial.println("Playorder:");
    for (int i = 0; i < maxSongs; i++) {
        Serial.println(playOrder[i]);
    }
}

String PlayOrderManager::getCurrentSong() {
    return filenames[playOrder[currentSong]];
}

void PlayOrderManager::nextSong() {
    currentSong++;
    if (currentSong >= maxSongs) {
        currentSong = 0; // Loop back to the start
    }
}

void PlayOrderManager::resetPlayOrder() {
    currentSong = 0;
    fillPlayOrderArray();
}