#include <WiFi.h>  // Include WiFi library for ESP32

const char* ssid     = "Ashwin";        // WiFi SSID
const char* password = "1234567890";    // WiFi password

const int TRIG_PIN = 12;     // Trigger pin
const int ECHO_PIN = 14;    // Echo pin

long duration, distance_cm;

WiFiServer server(80);  // Create a web server on port 80 (HTTP)

void setup() {
  Serial.begin(9600);      // Start serial communication

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);  // Connect to WiFi

  while (WiFi.status() != WL_CONNECTED) { // Wait until connected
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());  // Print local IP address

  server.begin();  // Start the server
}

void loop() {
  WiFiClient client = server.available();  // Check if a client has connected

  // Send ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo time
  duration = pulseIn(ECHO_PIN, HIGH);
  distance_cm = duration / 29 / 2; // Convert to cm

  if (client) {
    String currentRequest = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        currentRequest += c;

        // On receiving a full line (end of HTTP request line)
        if (c == '\n') {
          // Send standard HTTP response
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          // Send simple HTML content
          client.println("<!DOCTYPE html>");
          client.println("<html>");
          client.println("<meta http-equiv='refresh' content='2'>");
          client.println("<head><title>ULTRASONIC SENSOR DATA</title></head>");
          client.println("<p>DISTANCE</p>");
          client.print(distance_cm);
          client.println("</body></html>");

          break;  // Exit loop after sending response
        }
      }
    }

    delay(1);     
    client.stop(); // Close the connection
  }
}
