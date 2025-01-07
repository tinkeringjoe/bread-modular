#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>
#define TOTAL_PLAYERS 8

class Player {
  public:
    const uint8_t* sampleData = 0;
    uint16_t sampleLenght = 0;
    uint16_t index = 0;
};

Player players[TOTAL_PLAYERS];

uint8_t getPlayHead() {
  int16_t combinedValue = 0;
  int playCount = 0;

  for (int lc=0; lc<TOTAL_PLAYERS; lc++) {
    Player* player = &players[lc];
    if (player->sampleData != 0 && player->index < player->sampleLenght) {
      playCount += 1;
      combinedValue += (player->sampleData[player->index] - 127);
      player->index += 1;
    } else {
      player->sampleData = 0;
    }
  }

  return min(combinedValue + 127, 255);
}

void startPlayer(const uint8_t* sampleData, uint16_t sampleLength) {
  for (int lc=0; lc<TOTAL_PLAYERS; lc++) {
    if (players[lc].sampleData == 0) {
      players[lc].sampleData = sampleData;
      players[lc].sampleLenght = sampleLength;
      players[lc].index = 0;
      break;
    }
  }
}


#endif // PLAYER_H