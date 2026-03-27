#include <SPI.h>

#define CS 5
#define TRIG 12
#define ECHO 14

bool objectPresent = false;

unsigned long lastChangeTime = 0;
const int debounceDelay = 2000; // strong debounce

int entryCount = 0;
unsigned long address = 0;

bool systemReady = false;


void writeEnable() {
  digitalWrite(CS, LOW);
  SPI.transfer(0x06);
  digitalWrite(CS, HIGH);
}

void writeByte(uint32_t addr, byte data) {
  writeEnable();

  digitalWrite(CS, LOW);
  SPI.transfer(0x02);
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  SPI.transfer(data);
  digitalWrite(CS, HIGH);

  delay(3);
}

byte readByte(uint32_t addr) {
  digitalWrite(CS, LOW);
  SPI.transfer(0x03);
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  byte data = SPI.transfer(0x00);
  digitalWrite(CS, HIGH);
  return data;
}

void writeLog(String msg) {
  for (int i = 0; i < msg.length(); i++) {
    writeByte(address++, msg[i]);
  }
  writeByte(address++, '\n');
}

void readLogs() {
  Serial.println("\n==== FLASH LOG ====");
  for (uint32_t i = 0; i < address; i++) {
    Serial.print((char)readByte(i));
  }
  Serial.println("\n==== END ====");
}


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


String getTime() {
  unsigned long sec = millis() / 1000;

  int min = sec / 60;
  int s = sec % 60;

  char buf[10];
  sprintf(buf, "%02d:%02d", min, s);

  return String(buf);
}


void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  SPI.begin(18, 19, 23, CS);

  Serial.println("Starting...");
}


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

  if (inRange && !objectPresent && (now - lastChangeTime > debounceDelay)) {
    objectPresent = true;
    lastChangeTime = now;
    entryCount++;

    String msg = "ENTRY at: " + getTime();
    Serial.println(msg);
    writeLog(msg);

    if (entryCount >= 10) {
      Serial.println("\n>>> 10 Objects Detected! Reading Flash...\n");
      readLogs();
      entryCount = 0;
    }
  }

  else if (!inRange && objectPresent && (now - lastChangeTime > debounceDelay)) {
    objectPresent = false;
    lastChangeTime = now;

    String msg = "EXIT at: " + getTime();
    Serial.println(msg);
    writeLog(msg);
  }

  delay(100);
}
