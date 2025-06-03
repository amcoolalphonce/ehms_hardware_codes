#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <LiquidCrystal_I2C.h>

#define MAX30205_ADDRESS 0x48
#define TEMP_REGISTER 0x00
#define DATA_INTERVAL 10000  // 10 seconds in milliseconds

LiquidCrystal_I2C lcd(0x27, 16, 2);
PulseOximeter pox;

uint32_t lastUpdate = 0;
uint32_t lastDataCollection = 0;
float lastValidTemperature = 0;
bool temperatureError = false;

void onBeatDetected() {
  Serial.println("Beat Detected!");
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
    int16_t tempRaw = (msb << 8) | lsb;
    float temperature = tempRaw * 0.00390625;

    temperature += 2;
    
    // Basic sanity check for temperature values
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

void setup() {
  Serial.begin(9600);
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

  // Update pulse oximeter frequently
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    pox.update();
  }

  // Collect and display data every 10 seconds
  if (millis() - lastDataCollection > DATA_INTERVAL) {
    lastDataCollection = millis();

    float heartRate = pox.getHeartRate();
    float spo2 = pox.getSpO2();
    float temperature = readTemperature();

    Serial.print("HR: ");
    Serial.print(heartRate);
    Serial.print(" BPM | SpO2: ");
    Serial.print(spo2);
    Serial.print(" % | Temp: ");
    
    if (temperatureError) {
      if (lastValidTemperature != 0) {
        Serial.print(lastValidTemperature);
        Serial.println(" °C (last valid)");
        temperature = lastValidTemperature; // Use last valid reading
      } else {
        Serial.println("Error reading temp");
      }
    } else {
      Serial.print(temperature);
      Serial.println(" °C");
    }

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
    } else if (lastValidTemperature != 0) {
      lcd.print(lastValidTemperature, 1);
      lcd.print((char)223);
      lcd.print("C*");
    } else {
      lcd.print("Error");
    }
  }
}