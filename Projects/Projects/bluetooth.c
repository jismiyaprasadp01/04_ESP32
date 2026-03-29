#include "BluetoothSerial.h"  // Include Bluetooth Serial library
const int TRIG_PIN = 5;     // Trigger pin
const int ECHO_PIN = 18;    // Echo pin
BluetoothSerial SerialBT;     // Create an object of BluetoothSerial

void setup() {
  Serial.begin(9600);   
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);            // Start serial communication with PC 
  SerialBT.begin("ESP32_bluetooth");  // Start Bluetooth with device name "ESP32_bluetooth"
  Serial.println("Device started, now you can pair it with bluetooth!");
}

void loop() {
  long duration, distance_cm;

  // Send ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo time
  duration = pulseIn(ECHO_PIN, HIGH);
  distance_cm = duration / 29 / 2; // Convert to cm

  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  
  SerialBT.print("Distance: ") ;
  SerialBT.print(distance_cm);
  SerialBT.println(" cm");
  delay(20); 
}
