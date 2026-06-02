#include <Wire.h>

void receiveEvent(int howMany)
{
    Serial.print("Received: ");

    while(Wire.available())
    {
        char c = Wire.read();

        // Print only valid characters
        if(c >= 32 && c <= 126)
        {
            Serial.print(c);
        }
    }

    Serial.println();
}

void setup()
{
    Serial.begin(115200);

    Wire.begin(8);

    Wire.onReceive(receiveEvent);

    Serial.println("ESP32 Ready");
}

void loop()
{

}