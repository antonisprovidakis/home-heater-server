const char SOM = '<';
const char EOM = '>';
const char* EQUALS = "=";
const char* COMMA = ",";
const int BUFFER_SIZE = 128;

typedef struct {
  bool started;
  bool ended;
  char message[BUFFER_SIZE];
  byte index;
} MessageBuffer;

void receiveMessage(MessageBuffer *mb) {
  char inChar;
  while (Serial.available() > 0) {
    inChar = Serial.read();

    if (inChar == SOM) {
      mb->index = 0;
      mb->message[mb->index] = '\0';
      mb->started = true;
      mb->ended = false;
    }
    else if (inChar == EOM) {
      mb->ended = true;
      break;
    }
    else {
      if (mb->index < BUFFER_SIZE - 1) {
        mb->message[mb->index] = inChar;
        mb->index++;
        mb-> message[mb->index] = '\0';
      }
    }
  }
}

bool readyToParseMessage(MessageBuffer *mb) {
  return mb->started && mb->ended;
}

void resetMessageBuffer(MessageBuffer *mb) {
  mb->started = false;
  mb->ended = false;
  mb->index = 0;
  mb->message[mb->index] = '\0';
}
