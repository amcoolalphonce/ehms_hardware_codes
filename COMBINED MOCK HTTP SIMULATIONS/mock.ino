#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Mwende";
const char* password = "kavinya1662";


const char* server = "http://192.168.0.16:8000";
const char* endpoint = "/health-data/";

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float heartRate = 72.5;
    float spo2 = 98.0;
    float temperature = 36.6;
    
    // JSON payload
    StaticJsonDocument<200> doc;
    doc["heart_rate"] = heartRate;
    doc["spo2"] = spo2;
    doc["temperature"] = temperature;
    
    String payload;
    serializeJson(doc, payload);

    // Send HTTP POST request
    WiFiClient client;
    HTTPClient http;
    
    String url = String(server) + String(endpoint);
    Serial.print("Posting to: ");
    Serial.println(url);
    
    if (http.begin(client, url)) {
      http.addHeader("Content-Type", "application/json");
      
      int httpCode = http.POST(payload);
      
      if (httpCode > 0) {
        String response = http.getString();
        Serial.println("Response Code: " + String(httpCode));
        Serial.println("Response: " + response);
      } else {
        Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      
      http.end();
    } else {
      Serial.println("[HTTP] Unable to connect");
    }
  }
  
  delay(5000); // every 5 seconds
}
