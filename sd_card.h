const char* FILE_OK_SYMBOL = "~";
const char* DATA_FILE_NAME = "data.txt";

boolean initializeSD() {
  Serial.println(F("Initializing SD card..."));

  if (!SD.begin()) {
    Serial.println(F("SD card initialization failed"));
    return false;
  }

  Serial.println(F("SD card is ready to use."));
  return true;
}

boolean dataFileExists() {
  return SD.exists(DATA_FILE_NAME);
}

boolean removeDataFile() {
  if (!dataFileExists()) {
    return true; // file doesnt exist, we are OK to proceed
  }

  if (!SD.remove(DATA_FILE_NAME)) {
    Serial.println(F("Error while removing file."));
    return false;
  }

  Serial.println(F("File removed successfully."));
  return true;
}

File createFile(char *fileName) {
  File file = SD.open(fileName, FILE_WRITE);

  if (file) {
    Serial.println(F("File created successfully."));
  }
  else {
    Serial.println(F("Error while creating file."));
  }

  return file;
}

File createDataFile() {
  return createFile(DATA_FILE_NAME);
}

boolean writeToFile(File *file, char *text) {
  if (!file) {
    Serial.println(F("Couldn't write to file"));
    return false;
  }

  Serial.println("Writing to file: " + String(text));
  file->println(text);
  return true;
}

void closeFile(File *file) {
  if (file) {
    file->close();
    Serial.println(F("File closed"));
  }
}

File openFile(char *fileName) {
  File file = SD.open(fileName);

  if (file) {
    Serial.println(F("File opened with success!"));
  }
  else {
    Serial.println(F("Error opening file..."));
  }

  return file;
}

File openDataFile() {
  return openFile(DATA_FILE_NAME);
}

String readFileContent(File *file) {
  String line = "";

  while (file->available())  {
    line += (char)file->read();
  }

  return line;
}

boolean isDataFileOK(File *file) {
  if (file->seek(file->size() - 1)) {
    if ( ((char)file->peek()) == FILE_OK_SYMBOL[0]) {
      file->seek(0); // return index to start of file
      Serial.println(F("file OK"));
      return true;
    }
  }

  Serial.println(F("file NOT OK"));
  return false;
}

