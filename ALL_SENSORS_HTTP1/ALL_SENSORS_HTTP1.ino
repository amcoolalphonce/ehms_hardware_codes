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
const char* server = "http://192.168.0.23:8000";
// "http://192.168.83.150:8000"; // phone
// home wifi  http://192.168.0.16:8000" 
const char* endpoint = "/health-data/";

const int user_id = 22;
//1 AMCOOL
//2 PATIENT 1
//3 DRJAYV
//4 PATIENT2
//5 
//11 

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

// Variables for sensor readings
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

  // Initialize I2C and sensors
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

    // Read sensor data
    heartRate = pox.getHeartRate();
    spo2 = pox.getSpO2();
    temperature = readTemperature();

    // Display on serial monitor
    Serial.print("HR: ");
    Serial.print(heartRate);
    Serial.print(" BPM | SpO2: ");
    Serial.print(spo2);
    Serial.print(" % | Temp: ");
    if (temperature == 20) {
      Serial.println("Error reading temp");
    } else {
      Serial.print(temperature);
      Serial.println(" °C");
    }

    // Display on LCD
    updateLCD();

    // Post to server every thirty seconds (30,000ms)
    static uint32_t lastPostTime = 0;
    if (millis() - lastPostTime >= 30000) {
      lastPostTime = millis();
      postToServer();
    }
  }
}

void onBeatDetected() {
  Serial.println("Beat Detected!");
}

float readTemperature() {
  Wire.beginTransmission(MAX30205_ADDRESS);
  Wire.write(TEMP_REGISTER);
  if (Wire.endTransmission(false) != 0) {  
    Serial.println("Error communicating with MAX30205");
    return 20;
  }

  Wire.requestFrom(MAX30205_ADDRESS, 2);
  if (Wire.available() == 2) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    int16_t tempRaw = (msb << 8) | lsb;
    float temperature = tempRaw * 0.00390625; // Convert to Celsius
    //sanity check
      if (temperature > 45.0 || temperature < 20.0) {
      temperatureError = true;
      return 20;
    }
    temperatureError = false;
    lastValidTemperature = temperature;
    return temperature; 

  }
  temperatureError = true;
  return 20; // Communication error
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
  if (temperature != -999) {
    lcd.print(temperature, 1);
    lcd.print((char)223);
    lcd.print("C");
  } else {
    lcd.print("Error");
  }
}

void postToServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, cannot post data");
    return;
  }

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
      
      // Show success on LCD temporarily
      lcd.clear();
      lcd.print("Data sent!");
      delay(2000);
      updateLCD();
    } else {
      Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpCode).c_str());
      
      // Show error on LCD temporarily
      lcd.clear();
      lcd.print("Send failed!");
      delay(2000);
      updateLCD();
    }
    
    http.end();
  } else {
    Serial.println("[HTTP] Unable to connect");
  }
  // implement a function to discard further readings once its powered off.
}