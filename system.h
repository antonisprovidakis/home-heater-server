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
  bool enabled;
  Phase phase;
  boolean storageInit;
  Timer timer;
  MessageBuffer mb;
  Profile p;
} System;

///// Message Command Types /////
const char* HEATER_FUNCTION = "HF";
const char* PROFILE_UPDATE = "PU";
const char* DEBUG = "debug";
const char* CHECK_ENABLED = "check_enabled";
const char* REMOVE_DATA_FILE = "remove_data_file";

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

const int RELAY_PIN = 7;

void initTimer(Timer *timer) {
  timer->previousMillis = 0;
}

Profile createDefaultProfile() {
  // THIS IS THE DEFAULT PROFILE
  Profile defaultProfile = {
    .id = 1,
    .heat = 105 * MINUTE,
    .preserve = 2 * MINUTE,
    .rest = 2 * MINUTE
  };

  return defaultProfile;
}

void turnOffRelay(int relayPin) {
  digitalWrite(relayPin, HIGH); // HIGH = relay switch opens (current NOT passing)
}

void turnOnRelay(int relayPin) {
  digitalWrite(relayPin, LOW); // LOW = relay switch closes (current passing)
}

void disableSystem(System *sys) {
  turnOffRelay(RELAY_PIN);
  sys->enabled = false;
  Serial.println(F("System disabled..."));
}

void enableSystem(System *sys) {
  sys->enabled = true;
  Serial.println(F("System enabled..."));
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

void goToHeatPhase(System *sys) {
  sys->phase = HEAT;
  turnOnRelay(RELAY_PIN);
}

void goToPreservePhase(System *sys) {
  sys->phase = PRESERVE;
  turnOnRelay(RELAY_PIN);
}

void goToRestPhase(System *sys) {
  sys->phase = REST;
  turnOffRelay(RELAY_PIN);
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
      Serial.print(F("Unknown phase:"));
      Serial.println(phase);
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
        Serial.print(F("Unknown variable:"));
        Serial.println(nameToken);
      }
    }

    nameToken = strtok(NULL, EQUALS);
  }
}


// DOMAIN-SPECIFIC
void updateProfile(Profile *p, char *nameToken, char *valToken) {
  unsigned long int value = strtoul(valToken, NULL, 10);

  if (strcmp(nameToken, ID_TYPE) == 0) {
    p->id = (unsigned int) value;
    Serial.print(F("PROFILE_ID changed to:"));
    Serial.println(value);
  }
  else if (strcmp(nameToken, HEAT_TYPE) == 0) {
    p->heat = value;
    Serial.print(F("HEAT_INTERVAL changed to:"));
    Serial.println(value);
  }
  else if (strcmp(nameToken, PRESERVE_TYPE) == 0) {
    p->preserve = value;
    Serial.print(F("PRESERVE_INTERVAL changed to:"));
    Serial.println(value);
  }
  else if (strcmp(nameToken, REST_TYPE) == 0) {
    p->rest = value;
    Serial.print(F("REST_INTERVAL changed to:"));
    Serial.println(value);
  }
  else {
    Serial.print(F("Unknown variable type:"));
    Serial.println(nameToken);
  }
}

boolean isStorageInit(System *sys) {
  return sys->storageInit;
}

void writeProfileDataToFile(File *file, Profile *p) {
  file->print(F("id="));
  file->print(p->id);
  file->print(F(",heat="));
  file->print(p->heat);
  file->print(F(",preserve="));
  file->print(p->preserve);
  file->print(F(",rest="));
  file->print(p->rest);
  // write a special symbol '~' at the end of file to ensure file integrity
  file->print(FILE_INTEGRITY_SYMBOL);
}

boolean saveProfileData(System *sys) {
  if (!isStorageInit(sys)) {
    return false;
  }

  removeDataFile();

  File file = createDataFile();

  if (!file) {
    return false;
  }

  writeProfileDataToFile(&file, &sys->p);
  closeFile(&file);

  return true;
}

void processProfileUpdateMessage(char* message, System *sys) {
  char *nameToken = strtok(message, EQUALS);

  while (nameToken)  {
    char *valToken = strtok(NULL, COMMA);

    if (valToken) {
      updateProfile(&sys->p, nameToken, valToken);
    }

    nameToken = strtok(NULL, EQUALS);
  }
}

void debugSystemState(System *sys) {
  Serial.println(F("\n--- DEBUG ---"));

  Serial.print(F("Storage Initialized:"));
  Serial.println(sys->storageInit);
  Serial.print(F("System Enabled:"));
  Serial.println(sys->enabled);
  Serial.print(F("System Phase:"));
  Serial.println(sys->phase);
  Serial.print(F("Id:"));
  Serial.println(sys->p.id);
  Serial.print(F("Heat:"));
  Serial.println(sys->p.heat);
  Serial.print(F("Preserve:"));
  Serial.println(sys->p.preserve);
  Serial.print(F("Rest:"));
  Serial.println(sys->p.rest);
  Serial.println(F("--- END DEBUG ---\n"));
}

void checkEnabled(System *sys) {
  Serial.println(sys->enabled);
}

void processMessage(char* type, char* message, System *sys, boolean fromBT) {
  if (strcmp(type, HEATER_FUNCTION) == 0) {
    processHeaterFunctionMessage(message, sys);
  }
  else if (strcmp(type, PROFILE_UPDATE) == 0) {
    processProfileUpdateMessage(message, sys);
    if (fromBT) {
      saveProfileData(sys);
    }
  }
  else if (strcmp(type, DEBUG) == 0) {
    debugSystemState(sys);
  }
  else if (strcmp(type, CHECK_ENABLED) == 0) {
    checkEnabled(sys);
  }
  else if (strcmp(type, REMOVE_DATA_FILE) == 0) {
    removeDataFile();
  }
  else {
    Serial.print(F("Unknown message type:"));
    Serial.println(type);
  }
}

boolean updateProfileFromDataFile(System *sys) {
  File file = openDataFile();

  if (!file) {
    return false;
  }

  if (!isDataFileOK(&file)) {
    return false;
  }

  String content = readFileContent(&file);
  closeFile(&file);

  content.remove(content.length() - 1); // remove integrity character: "~"

  char *temp = content.c_str();
  char message[strlen(temp) + 1];
  strcpy(message, temp);

  processMessage(PROFILE_UPDATE, message, sys, false);

  return true;
}

void initProfile(System *sys) {
  Profile *p = &sys->p;

  if (!isStorageInit(sys)) {
    *p = createDefaultProfile();
    Serial.println(F("Default profile loaded. Storage not init!"));
    return;
  }

  if (!dataFileExists()) {
    *p = createDefaultProfile();
    saveProfileData(sys);
    Serial.println(F("Default profile loaded. Saved to SD-Card"));
    return;
  }

  updateProfileFromDataFile(sys);
  Serial.println(F("Profile loaded from SD-Card"));
}

void initSystem(System *sys) {
  Serial.println(F("--- Init System ---"));
  sys->storageInit = initializeSD();
  initProfile(sys);
  initTimer(&sys->timer);
  pinMode(RELAY_PIN, OUTPUT);
  enableSystem(sys);
  goToPhase(HEAT, sys); // defaults to Heat phase
  Serial.println(F("--- End System Init ---"));
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

      processMessage(commandType, message, sys, true);
    }
  }
}

void heaterLoop(System * sys) {
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

