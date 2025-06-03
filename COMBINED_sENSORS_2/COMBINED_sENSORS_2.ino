#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <LiquidCrystal_I2C.h>

// LCD I2C Address (usually 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pulse Oximeter object
PulseOximeter pox;

// Timing Variables
uint32_t lastUpdate = 0;

// Temperature sensor definitions
#define MAX30205_ADDRESS 0x48
#define TEMP_REGISTER 0x00

// Callback function to print beat detection
void onBeatDetected() {
    Serial.println("Beat Detected!");
}

void setup() {
    Serial.begin(9600);
    Wire.begin(); // Initialize I2C

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");

    // Initialize MAX30100
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
    // Update pulse oximeter data
    pox.update();

    // Print every 1 second
    if (millis() - lastUpdate > 1000) {
        lastUpdate = millis();

        float heartRate = pox.getHeartRate();
        float spo2 = pox.getSpO2();

        Serial.print("Heart Rate: ");
        Serial.print(heartRate);
        Serial.print(" BPM  |  SpO2: ");
        Serial.print(spo2);
        Serial.println(" %");

        // Read temperature from MAX30205
        Wire.beginTransmission(MAX30205_ADDRESS);
        Wire.write(TEMP_REGISTER);
        Wire.endTransmission();

        Wire.requestFrom(MAX30205_ADDRESS, 2);
        float temperature = 0;
        if (Wire.available() == 2) {
            byte msb = Wire.read();
            byte lsb = Wire.read();
            int16_t tempRaw = (msb << 8) | lsb;
            temperature = tempRaw * 0.00390625;
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println(" °C");
        }

        // Display on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("HR:");
        lcd.print(heartRate);
        lcd.print(" SpO2:");
        lcd.print(spo2);

        lcd.setCursor(0, 1);
        lcd.print("Temp:");
        lcd.print(temperature);
        lcd.print(" C");
    }
}
