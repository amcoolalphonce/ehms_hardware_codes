#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <LiquidCrystal_I2C.h>

// Define LCD with I2C Address (usually 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Create Pulse Oximeter Object
PulseOximeter pox;

// Timing Variables
uint32_t lastUpdate = 0;

// Callback function to print readings
void onBeatDetected() {
    Serial.println("Beat Detected!");
}

void setup() {
    Serial.begin(115200);
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
    // Read sensor data
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

        // Display on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("HR: ");
        lcd.print(heartRate);
        lcd.print(" BPM");

        lcd.setCursor(0, 1);
        lcd.print("SpO2: ");
        lcd.print(spo2);
        lcd.print(" %");
    }
}
