#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Mwende";
const char* password = "kavinya1661";


const char* server = "http://192.168.0.16:8000";  // local network IP 
const char* endpoint = "/health-data/";


const int user_id = 1; // assign each time

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
    //random values
    float heartRate = random(60, 101);  
    float spo2 = random(950, 1001) / 10.0;  
    float temperature = random(361, 373) / 10.0;
    
    // Create JSON payload
    StaticJsonDocument<200> doc;
    doc["user_id"] = user_id; 
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
  
  delay(60000); // Send data every 1 minute
}
