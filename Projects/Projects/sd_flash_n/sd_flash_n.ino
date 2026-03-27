#include <SPI.h>
#include <SD.h>

// ---------- PINS ----------
#define CS_SD 5
#define CS_FLASH 15
#define TRIG 12
#define ECHO 14

// ---------- VARIABLES ----------
bool objectPresent = false;
unsigned long lastChangeTime = 0;
const int debounceDelay = 2000;

unsigned long address = 0;

// ---------- SPI CONTROL ----------
void selectSD() {
  digitalWrite(CS_FLASH, HIGH);
  digitalWrite(CS_SD, LOW);
}

void selectFlash() {
  digitalWrite(CS_SD, HIGH);
  digitalWrite(CS_FLASH, LOW);
}

void deselectAll() {
  digitalWrite(CS_SD, HIGH);
  digitalWrite(CS_FLASH, HIGH);
}

// ---------- TIME ----------
String getTime() {
  unsigned long sec = millis() / 1000;

  int h = sec / 3600;
  int m = (sec % 3600) / 60;
  int s = sec % 60;

  char buf[12];
  sprintf(buf, "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

// ---------- ULTRASONIC ----------
float getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  return duration * 0.034 / 2;
}

// ---------- FLASH ----------
void flashWriteEnable() {
  selectFlash();
  SPI.transfer(0x06);
  deselectAll();
}

void flashWriteByte(uint32_t addr, byte data) {
  flashWriteEnable();

  selectFlash();

  SPI.transfer(0x02);
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  SPI.transfer(data);

  deselectAll();
  delay(3);
}

byte flashReadByte(uint32_t addr) {
  selectFlash();

  SPI.transfer(0x03);
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);

  byte data = SPI.transfer(0x00);

  deselectAll();
  return data;
}

void eraseFlash() {
  Serial.println("Erasing Flash...");

  flashWriteEnable();

  selectFlash();
  SPI.transfer(0xC7); // chip erase
  deselectAll();

  delay(5000);

  Serial.println("Flash Erased");
}

void flashWriteLog(String msg) {
  for (int i = 0; i < msg.length(); i++) {
    flashWriteByte(address++, msg[i]);
  }
  flashWriteByte(address++, '\n');
}

void readFlashLogs() {
  Serial.println("\n==== FLASH LOG ====");

  for (uint32_t i = 0; i < address; i++) {
    Serial.print((char)flashReadByte(i));
  }

  Serial.println("\n==== END ====");
}

// ---------- SD ----------
void writeSD(String msg) {
  selectSD();

  File file = SD.open("/log.txt", FILE_APPEND);

  if (file) {
    file.println(msg);
    file.close();
  } else {
    Serial.println("SD Write Error!");
  }

  deselectAll();
}

void readSD() {
  selectSD();

  Serial.println("\n==== SD LOG ====");

  File file = SD.open("/log.txt", FILE_READ);

  if (!file) {
    Serial.println("File open failed!");
    deselectAll();
    return;
  }

  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();

  Serial.println("\n==== END ====");

  deselectAll();
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(CS_SD, OUTPUT);
  pinMode(CS_FLASH, OUTPUT);

  deselectAll();

  SPI.begin(18, 19, 23);

  // SD INIT
  selectSD();
  if (!SD.begin(CS_SD)) {
    Serial.println("SD Card Failed!");
  } else {
    Serial.println("SD Card Ready");
  }
  deselectAll();

  // FLASH INIT
  address = 0;
  eraseFlash();

  Serial.println("System Ready");
}

// ---------- LOOP ----------
void loop() {

  // UART CONTROL
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == '0') {
      readSD();
    }
    else if (cmd == '1') {
      readFlashLogs();
    }
  }

  // SENSOR
  float d = getDistance();
  bool inRange = (d > 5 && d < 30);

  String time = getTime();

  // ENTRY
  if (inRange && !objectPresent && (millis() - lastChangeTime > debounceDelay)) {
    objectPresent = true;
    lastChangeTime = millis();

    String msg = "ENTRY at: " + time;

    Serial.println(msg);

    writeSD(msg);
    flashWriteLog(msg);
  }

  // EXIT
  else if (!inRange && objectPresent && (millis() - lastChangeTime > debounceDelay)) {
    objectPresent = false;
    lastChangeTime = millis();

    String msg = "EXIT at: " + time;

    Serial.println(msg);

    writeSD(msg);
    flashWriteLog(msg);
  }

  delay(200);
}