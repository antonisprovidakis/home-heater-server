#include "bluetooth.h"
#include "system.h"

System sys;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("--- Init System ---");
  initSystem(&sys);
  Serial.println("--- End init ---");
}

void loop() {
  receiveMessage(&sys.mb);

  if (readyToParseMessage(&sys.mb)) {
    parseMessage(&sys);
    resetMessageBuffer(&sys.mb);
  }

  heaterLoop(&sys);
}


