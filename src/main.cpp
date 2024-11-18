#include <Arduino.h>
#include "w25q.h"

#define W25Q_MISO    12
#define W25Q_MOSI    13
#define W25Q_SCK     14
#define W25Q_CS      15


W25Q w25q16(W25Q_SCK, W25Q_MISO, W25Q_MOSI, W25Q_CS);

// put function declarations here:

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  w25q16.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}

