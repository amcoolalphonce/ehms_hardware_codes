#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "MMS";
const char* password = "Paris720";

// Server details
//const char* server = "http://192.168.83.150:8000"; phone internet
const char* server = "http:// 192.168.0.23:8000";
const char* endpoint = "/health-data/";

const int user_id = 8;

// Sensor constants
#define MAX30205_ADDRESS 0x48
#define TEMP_REGISTER 0x00
bool temperatureError = false;
float lastValidTemperature = 0;

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pulse oximeter
PulseOximeter pox;
uint32_t lastUpdate = 0;

// Sensor reading struct
struct SensorData {
  float heartRate;
  float spo2;
  float temperature;
};

SensorData readings[3];  // buffer for last 3 valid readings
int readingIndex = 0;
int validCount = 0;

// Current reading
float heartRate = 0;
float spo2 = 0;
float temperature = 0;

void setup() {
  Serial.begin(9600);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  if (!pox.begin()) {
    Serial.println("MAX30100 INIT FAILED!");
    lcd.clear();
    lcd.print("MAX30100 Failed");
    while (1);
  } else {
    Serial.println("MAX30100 Initialized");
    lcd.clear();
    lcd.print("MAX30100 Ready");
  }

  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
  pox.update();

  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();

    // Read sensors
    heartRate = pox.getHeartRate();
    spo2 = pox.getSpO2();
    float tempRead = readTemperature();
    
    // Use previous valid temperature if reading is bad
    if (temperatureError) {
      temperature = lastValidTemperature;
    } else {
      temperature = tempRead;
    }

    Serial.print("HR: "); Serial.print(heartRate);
    Serial.print(" | SpO2: "); Serial.print(spo2);
    Serial.print(" | Temp: ");
    if (temperatureError) Serial.println("Error (using last)");
    else Serial.println(temperature);

    updateLCD();

    // Store reading in circular buffer
    if (!temperatureError) {
      readings[readingIndex] = {heartRate, spo2, temperature};
      readingIndex = (readingIndex + 1) % 3;
      if (validCount < 3) validCount++;
    }

    // Only post to server if we have 3 valid entries
    static uint32_t lastPostTime = 0;
    if (validCount == 3 && (millis() - lastPostTime >= 30000)) {
      lastPostTime = millis();

      // Choose the middle (median) value of the three for each metric
      SensorData finalData = selectBestReading();

      postToServer(finalData);
    }
  }
}

float readTemperature() {
  Wire.beginTransmission(MAX30205_ADDRESS);
  Wire.write(TEMP_REGISTER);
  if (Wire.endTransmission(false) != 0) {
    Serial.println("Error communicating with MAX30205");
    temperatureError = true;
    return 20;
  }

  Wire.requestFrom(MAX30205_ADDRESS, 2);
  if (Wire.available() == 2) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    int16_t raw = (msb << 8) | lsb;
    float temp = raw * 0.00390625;

    if (temp > 45.0 || temp < 20.0) {
      temperatureError = true;
      return 20;
    }

    temperatureError = false;
    lastValidTemperature = temp;
    return temp;
  }

  temperatureError = true;
  return 20;
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HR:");
  lcd.print(heartRate, 0);
  lcd.print(" SpO2:");
  lcd.print(spo2, 0);

  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  if (!temperatureError) {
    lcd.print(temperature, 1);
    lcd.print((char)223);
    lcd.print("C");
  } else {
    lcd.print("Error");
  }
}

void postToServer(SensorData data) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, cannot post data");
    return;
  }

  StaticJsonDocument<200> doc;
  doc["user_id"] = user_id;
  doc["heart_rate"] = data.heartRate;
  doc["spo2"] = data.spo2;
  doc["temperature"] = data.temperature;

  String payload;
  serializeJson(doc, payload);

  WiFiClient client;
  HTTPClient http;
  String url = String(server) + String(endpoint);
  Serial.print("Posting to: "); Serial.println(url);

  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(payload);

    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("Response Code: " + String(httpCode));
      Serial.println("Response: " + response);
      lcd.clear(); lcd.print("Data sent!");
    } else {
      Serial.println("[HTTP] POST failed: " + http.errorToString(httpCode));
      lcd.clear(); lcd.print("Send failed!");
    }

    delay(2000);
    updateLCD();
    http.end();
  } else {
    Serial.println("[HTTP] Unable to connect");
  }
}

// Select the "middle" reading of the 3 sets for stability
SensorData selectBestReading() {
  SensorData sorted[3];
  memcpy(sorted, readings, sizeof(readings));

  // Simple bubble sort by temperature (can sort by any metric)
  for (int i = 0; i < 2; ++i) {
    for (int j = i + 1; j < 3; ++j) {
      if (sorted[i].temperature > sorted[j].temperature) {
        SensorData temp = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = temp;
      }
    }
  }

  return sorted[1]; // Return the median (middle) set
}

void onBeatDetected() {
  Serial.println("Beat Detected!");
}
