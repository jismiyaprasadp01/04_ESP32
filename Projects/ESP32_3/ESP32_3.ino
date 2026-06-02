#include <HardwareSerial.h>

HardwareSerial sim800(2);  // UART2

void setup() {
  Serial.begin(115200);
  sim800.begin(9600, SERIAL_8N1, 16, 17); // RX, TX

  delay(5000);  // Give time for network

  Serial.println("SIM800L Testing...");

  sendAT("AT");          // Check module
  sendAT("AT+CPIN?");    // SIM status
  sendAT("AT+CREG?");    // Network
  sendAT("AT+CSQ");      // Signal

  sendAT("AT+CMGF=1");   // SMS text mode

  // Send SMS
  sim800.println("AT+CMGS=\"YOUR_NUMBER\"");
  delay(1000);

  sim800.print("Hello from ESP32 + SIM800L");
  delay(500);

  sim800.write(26); // CTRL+Z  
  Serial.println("SMS command sent");
}

void loop() 
{
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
}

void sendAT(String cmd) {
  sim800.println(cmd);
  delay(1000);

  while (sim800.available()) {
    Serial.write(sim800.read());
  }
}
