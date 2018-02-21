#include "bluetooth.h"
#include "system.h"


System sys;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("--- Init ---");
  initSystem(&sys);
  enableSystem(&sys);
  turnOnRelay(sys.relayPin);
  Serial.println("--- End init ---");
}

void loop() {
  Serial.println("--- loop started ---");
  receiveMessage(&sys.mb);

  if (readyToParseMessage(&sys.mb)) {
    Serial.println("--- in readyToParseMessage block ---");
    parseMessage(&sys);
    resetMessageBuffer(&sys.mb);
  }

  
  debugSystemState(&sys);
  delay(15000);
//  heaterLoop(&sys);
}


