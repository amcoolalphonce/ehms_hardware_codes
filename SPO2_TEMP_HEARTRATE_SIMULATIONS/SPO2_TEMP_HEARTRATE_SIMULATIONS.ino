#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD at address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Simulate temperature values between 34.8 and 37.4
float simulateTemperature() {
  return random(348, 374) / 10.0;
}

// Example simulated heart rate and SpO2 values (replace with your real sensor code)
int heartRate = 0;
int spO2 = 0;

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Simulated sensors setup (replace with your sensors)
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  // Simulate reading heart rate and SpO2 (replace with actual sensor reading)
  heartRate = random(60, 100);
  spO2 = random(95, 100);

  float temperature = simulateTemperature();

  // Display on Serial Monitor (optional)
  Serial.print("HR: "); Serial.print(heartRate);
  Serial.print(" bpm, SpO2: "); Serial.print(spO2);
  Serial.print(" %, Temp: "); Serial.print(temperature);
  Serial.println(" C");

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("HR:"); lcd.print(heartRate);
  lcd.print(" SpO2:"); lcd.print(spO2);

  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.print(temperature, 1);
  lcd.print((char)223); // degree symbol
  lcd.print("C   ");

  delay(5000); // Update every 5 seconds
}
