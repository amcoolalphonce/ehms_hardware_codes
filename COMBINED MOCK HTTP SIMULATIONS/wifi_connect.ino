#include <ESP8266WiFi.h>

const char* ssid = "Mwende"; 
const char* password = "kavinya1661"; 

void setup() {
    Serial.begin(9600);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());  // assigned IP address
}

void loop() {
    // Keep the connection alive
}
