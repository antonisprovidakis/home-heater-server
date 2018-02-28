#include <SD.h>
#include <SPI.h>
#include "bluetooth.h"
#include "sd_card.h"
#include "system.h"

System sys;

void setup() {
  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB
//  }

  initSystem(&sys);
}

void loop() {
  receiveMessage(&sys.mb);

  if (readyToParseMessage(&sys.mb)) {
    parseMessage(&sys);
    resetMessageBuffer(&sys.mb);
  }

  heaterLoop(&sys);
}


