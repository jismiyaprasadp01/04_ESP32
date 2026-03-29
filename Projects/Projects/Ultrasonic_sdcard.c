#include <SPI.h>
#include <SD.h>

#define CS 5
#define TRIG 12
#define ECHO 14

bool objectPresent = false;
unsigned long lastChangeTime = 0;
const int debounceDelay = 2000;

int entryCount = 0;
bool systemReady = false;

File logFile;

// ---------- SENSOR ----------

float getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  return duration * 0.034 / 2;
}

float getStableDistance() {
  float sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += getDistance();
    delay(5);
  }
  return sum / 5.0;
}

// ---------- TIME ----------

String getTime() {
  unsigned long sec = millis() / 1000;
  int min = sec / 60;
  int s = sec % 60;

  char buf[10];
  sprintf(buf, "%02d:%02d", min, s);
  return String(buf);
}

// ---------- LOG FUNCTIONS ----------

void writeLog(String msg) {
  logFile = SD.open("/log.txt", FILE_APPEND);
  if (logFile) {
    logFile.println(msg);
    logFile.close();
  } else {
    Serial.println("Error opening file!");
  }
}

void readLogs() {
  logFile = SD.open("/log.txt");
  if (logFile) {
    Serial.println("\n==== SD LOG ====");
    while (logFile.available()) {
      Serial.write(logFile.read());
    }
    logFile.close();
    Serial.println("\n==== END ====");
  } else {
    Serial.println("Error reading file!");
  }
}

// ---------- SETUP ----------

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  if (!SD.begin(CS)) {
    Serial.println("SD Card Failed!");
    while (1);
  }

  Serial.println("SD Card Ready");
}

// ---------- LOOP ----------

void loop() {
  unsigned long now = millis();

  if (!systemReady) {
    if (now > 3000) {
      systemReady = true;
      Serial.println("System Ready");
    }
    return;
  }

  float d = getStableDistance();
  bool inRange = (d > 5 && d < 30);

  // ENTRY
  if (inRange && !objectPresent && (now - lastChangeTime > debounceDelay)) {
    objectPresent = true;
    lastChangeTime = now;
    entryCount++;

    String msg = "ENTRY at: " + getTime();
    Serial.println(msg);
    writeLog(msg);

    if (entryCount >= 10) {
      readLogs();
      entryCount = 0;
    }
  }

  // EXIT
  else if (!inRange && objectPresent && (now - lastChangeTime > debounceDelay)) {
    objectPresent = false;
    lastChangeTime = now;

    String msg = "EXIT at: " + getTime();
    Serial.println(msg);
    writeLog(msg);
  }

  delay(1000);
}
