#include <CRC32.h>

#include <GyverMAX7219.h>

MAX7219 < 4, 4, 9 > mtrx;   // (matrixByVerticalt x matrixByHorisontal), CS D9

// Number of 8x8 segments you are connecting
const byte W_LEDS = 32;
const byte H_LEDS = 32;
byte level[H_LEDS][W_LEDS];
byte temp[5][W_LEDS];
CRC32 crc;
int cycleCounter = 0;
uint32_t lastCrc;

void generateRandom() {
  for (byte i = 0; i < H_LEDS; i++) {
    for (byte k = 0; k < W_LEDS; k++) {
      if (random(0, 2) == 1) {
        level[i][k] = random(0, 2);
        mtrx.dot(k, i, level[i][k]);
      }
    }
  }
}

void generateCopperHead() {
  level[15][8] = 1;
  level[16][8] = 1;
  level[15][9] = 1;
  level[16][9] = 1;
  level[14][11] = 1;
  level[15][11] = 1;
  level[16][11] = 1;
  level[17][11] = 1;
  level[13][12] = 1;
  level[14][12] = 1;
  level[17][12] = 1;
  level[18][12] = 1;
  level[12][13] = 1;
  level[19][13] = 1;
  level[12][15] = 1;
  level[19][15] = 1;
  level[12][16] = 1;
  level[19][16] = 1;
  level[14][16] = 1;
  level[17][16] = 1;
  level[15][17] = 1;
  level[16][17] = 1;
  level[15][18] = 1;
  level[16][18] = 1;
  level[14][19] = 1;
  level[13][19] = 1;
  level[17][19] = 1;
  level[18][19] = 1;
  for (byte i = 0; i < H_LEDS; i++) {
    for (byte k = 0; k < W_LEDS; k++) {
      mtrx.dot(k, i, level[i][k]);
    }
  }

  mtrx.update();
}

void render() {
  for (byte x = 0; x < W_LEDS; x++) {
    temp[0][x] = level[0][x];
    temp[1][x] = level[31][x];
  }
  for (byte y = 0; y < H_LEDS; y++) {
    for (byte x = 0; x < W_LEDS; x++) {
      if (y == 0) {
        temp[2][x] = temp[1][x];
        temp[3][x] = level[y][x];
        temp[4][x] = level[y + 1][x];
      } else if (y == 31) {
        temp[2][x] = temp[3][x];
        temp[3][x] = temp[4][x];
        temp[4][x] = temp[0][x];
      } else {
        temp[2][x] = temp[3][x];
        temp[3][x] = temp[4][x];
        temp[4][x] = level[y + 1][x];
      }
    }
    for (byte x = 0; x < W_LEDS; x++) {
      int count = getCountOfNeighbors(x);
      if (temp[3][x] == 0 && count == 3) {
        level[y][x] = 1;
        mtrx.dot(x, y, 1);
      } else if (temp[3][x] == 1 && (count > 3 || count < 2)) {
        level[y][x] = 0;
        mtrx.dot(x, y, 0);
      }
    }
  }
  mtrx.update();
}

int getCountOfNeighbors(int x) {
  int xLength = 31;
  int west = temp[3][x - 1 < 0 ? xLength : x - 1];//left
  int east = temp[3][x + 1 > xLength ? 0 : x + 1];//right
  int north = temp[2][x];//up
  int south = temp[4][x];//down
  int northWest = temp[2][x - 1 < 0 ? xLength : x - 1];//leftUp
  int southWest = temp[4][x - 1 < 0 ? xLength : x - 1];//leftDown
  int northEast = temp[2][x + 1 > xLength ? 0 : x + 1];////rightUp
  int southEast = temp[4][x + 1 > xLength ? 0 : x + 1];//rightDown
  return west + east + north + south + northWest + southWest + northEast + southEast;
}

void setup() {
  mtrx.begin();
  mtrx.setBright(5);  //0..15
  mtrx.setType(1);
  Serial.begin(9600);
  randomSeed(analogRead(A1));
  if (random(0, 2) == 1) {
    generateCopperHead();
  } else {
      generateRandom();
    }
  delay(1000);

}

void loop() {
  render();
  for (int y = 0; y < H_LEDS; y++) {
    for (int x = 0; x < W_LEDS; x++) {
      crc.update(level[y][x]);
    }
  }
  uint32_t tmpCrc = crc.finalize();
  if (lastCrc == tmpCrc) {
    if (cycleCounter == 5) {
      cycleCounter = 0;
      generateRandom();
    } else {
      cycleCounter++;
      if (cycleCounter > 5) {
        cycleCounter = 0;
      }
    }
  } else {
    lastCrc = tmpCrc;
    cycleCounter++;
  }
  crc.reset();
  delay(100);
}
