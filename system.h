enum Phase {
  HEAT = 1,
  PRESERVE,
  REST
};

typedef struct {
  unsigned long int previousMillis;
  unsigned long int currentMillis;
} Timer;

typedef struct {
  unsigned int id;
  unsigned long int heat;
  unsigned long int preserve;
  unsigned long int rest;
} Profile;

typedef struct {
  Timer timer;
  bool enabled;
  Phase phase;
  int relayPin;
  MessageBuffer mb;
  Profile p;
} System;

///// Message Command Types /////
const char* HEATER_FUNCTION = "HF";
const char* PROFILE_UPDATE = "PU";
const char* DEBUG = "debug";

///// Interval Type Constants /////
const char* ID_TYPE = "id";
const char* HEAT_TYPE = "heat";
const char* PRESERVE_TYPE = "preserve";
const char* REST_TYPE = "rest";

///// Time Constants /////
const unsigned long SECOND = 1000;
const unsigned long MINUTE = 60 * SECOND;
const unsigned long HOUR = 60 * MINUTE;

///// Function Type Constant /////
const char* ENABLED_TYPE = "enabled";
const char* PHASE_TYPE = "phase";

void initTimer(Timer *timer) {
  timer->previousMillis = 0;
}

void initProfile(Profile *p) {
  //  TODO: read from sd card
  // if data found on SD-Card
  //   read from ther
  // else

  // TODO: change to default profile on APP
  p->id = 1;
  p->heat = 3 * SECOND;
  p->preserve = 2 * SECOND;
  p->rest = 1 * SECOND;
}

// DOMAIN-SPECIFIC
void updateProfile(Profile *p, char *nameToken, char *valToken) {
  unsigned long int value = strtoul(valToken, NULL, 10);

  if (strcmp(nameToken, ID_TYPE) == 0) {
    p->id = (unsigned int) value;
    Serial.println("ProfileID changed to: : " + String(value));
  }
  else if (strcmp(nameToken, HEAT_TYPE) == 0) {
    p->heat = value;
    Serial.println("HEAT_INTERVAL changed to: : " + String(value));
  }
  else if (strcmp(nameToken, PRESERVE_TYPE) == 0) {
    p->preserve = value;
    Serial.println("PRESERVE_INTERVAL changed to: : " + String(value));
  }
  else if (strcmp(nameToken, REST_TYPE) == 0) {
    p->rest = value;
    Serial.println("REST_INTERVAL changed to: : " + String(value));
  }
  else {
    Serial.println("Unknown variable type:" + String(nameToken));
  }
}

void turnOffRelay(int relayPin) {
  // HIGH = relay switch opens (current NOT passing)
  digitalWrite(LED_BUILTIN, LOW); // DEBUG

  //  digitalWrite(LED_BUILTIN, HIGH);
  //  digitalWrite(relayPin, HIGH);
}

void turnOnRelay(int relayPin) {
  // LOW = relay switch closes (current passing)
  digitalWrite(LED_BUILTIN, HIGH); // DEBUG

  //  digitalWrite(LED_BUILTIN, LOW);
  //  digitalWrite(relayPin, LOW);
}

void disableSystem(System *sys) {
  turnOffRelay(&sys->relayPin);
  sys->enabled = false;
}

void enableSystem(System *sys) {
  sys->enabled = true;
}

void goToHeatPhase(System *sys) {
  sys->phase = HEAT;
  turnOnRelay(sys->relayPin);
}

void goToPreservePhase(System *sys) {
  sys->phase = PRESERVE;
  turnOnRelay(sys->relayPin);
}

void goToRestPhase(System *sys) {
  sys->phase = REST;
  turnOffRelay(sys->relayPin);
}

void goToPhase(Phase phase, System *sys) {
  switch (phase) {
    case HEAT:
      goToHeatPhase(sys);
      break;
    case PRESERVE:
      goToPreservePhase(sys);
      break;
    case REST:
      goToRestPhase(sys);
      break;
    default:
      Serial.println("Unknown phase:" + String(phase));
  }
}

void initSystem(System *sys) {
  initProfile(&sys->p);
  initTimer(&sys->timer);
  sys->relayPin = 7; // default relay pin
  enableSystem(sys);
  goToPhase(HEAT, sys); // default to Heat phase
}

Phase stringToPhaseType(char *typeAsString) {
  if (strcmp(typeAsString, HEAT_TYPE) == 0) {
    return HEAT;
  }
  else if (strcmp(typeAsString, PRESERVE_TYPE) == 0) {
    return PRESERVE;
  }
  else if (strcmp(typeAsString, REST_TYPE) == 0) {
    return REST;
  }
  else {
    return NULL;
  }
}

void processHeaterFunctionMessage(char* message, System *sys) {
  char *nameToken = strtok(message, EQUALS);
  while (nameToken)  {
    char *valToken = strtok(NULL, COMMA);

    if (valToken) {
      if (strcmp(nameToken, ENABLED_TYPE) == 0) {
        boolean enabled = strcmp(valToken, "true") == 0 ? true : false;

        if (enabled) {
          enableSystem(sys);
        }
        else {
          disableSystem(sys);
        }
      }
      else if (strcmp(nameToken, PHASE_TYPE) == 0) {
        Phase phase = stringToPhaseType(valToken);
        if (phase) {
          goToPhase(phase, sys);
        }
      }
      else {
        Serial.println("Unknown variable:" + String(nameToken));
      }
    }

    nameToken = strtok(NULL, EQUALS);
  }
}

void processProfileActivationMessage(char* message, Profile *p) {
  char *nameToken = strtok(message, EQUALS);

  while (nameToken)  {
    char *valToken = strtok(NULL, COMMA);

    if (valToken) {
      updateProfile(p, nameToken, valToken);
    }

    nameToken = strtok(NULL, EQUALS);
  }
}

void debugSystemState(System *sys) {
  Serial.println("\n--- DEBUG ---");
  Serial.println("System Enabled:" + String(sys->enabled));
  Serial.println("System Phase:" + String(sys->phase));
  Serial.println("Id:" + String(sys->p.id) );
  Serial.println("Heat:" + String(sys->p.heat));
  Serial.println("Preserve:" + String(sys->p.preserve));
  Serial.println("Rest:" + String(sys->p.rest));
  Serial.println("--- END DEBUG ---\n");
}

void processMessage(char* type, char* message, System *sys) {
  if (strcmp(type, HEATER_FUNCTION) == 0) {
    processHeaterFunctionMessage(message, sys);
  }
  else if (strcmp(type, PROFILE_UPDATE) == 0) {
    processProfileActivationMessage(message, &sys->p);
  }
  else if (strcmp(type, DEBUG) == 0) {
    debugSystemState(sys);
  }
  else {
    Serial.println("Unknown message type:" + String(type));
  }
}

// Message of form: <command:commandType,name1=value1,name2=value2>
void parseMessage(System *sys) {
  char message[strlen(sys->mb.message) + 1];
  strcpy(message, sys->mb.message);

  char *commandToken = strtok(message, EQUALS);

  if (commandToken && strcmp(commandToken, "command") == 0) {
    char *commandTypeToken = strtok(NULL, COMMA);

    if (commandTypeToken) {
      char commandType[strlen(commandTypeToken) + 1];
      strcpy(commandType, commandTypeToken);

      char* messageWithoutCommandType = strchr(sys->mb.message, COMMA[0]) + 1;
      strcpy(message, messageWithoutCommandType);

      processMessage(commandType, message, sys);
    }
  }
}

void heaterLoop(System *sys) {
  if (sys->enabled) {
    sys->timer.currentMillis = millis();

    // HEAT phase
    if (sys->phase == HEAT && sys->timer.currentMillis - sys->timer.previousMillis >= sys->p.heat) {
      // if heat interval has been pass, go to rest phase
      sys->timer.previousMillis = sys->timer.currentMillis;
      goToPhase(REST, sys);
    }

    // PRESERVE phase
    if (sys->phase == PRESERVE && sys->timer.currentMillis - sys->timer.previousMillis >= sys->p.preserve) {
      // if preserve interval has been pass, go to rest phase
      sys->timer.previousMillis = sys->timer.currentMillis;
      goToPhase(REST, sys);
    }

    // REST phase
    if (sys->phase == REST && sys->timer.currentMillis - sys->timer.previousMillis >= sys->p.rest) {
      // if rest interval has been pass, go to preserve phase
      sys->timer.previousMillis = sys->timer.currentMillis;
      goToPhase(PRESERVE, sys);
    }
  }
}


