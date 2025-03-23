#include <Wire.h>
#define MAX30205_ADDRESS 0x48
#define TEMP_REGISTER 0x00

void setup() {
  Wire.begin(D2, D1); // SDA, SCL
  Serial.begin(9600);
}

void loop() {
  Wire.beginTransmission(MAX30205_ADDRESS);
  Wire.write(TEMP_REGISTER);
  Wire.endTransmission();
  
  Wire.requestFrom(MAX30205_ADDRESS, 2);
  if (Wire.available() == 2) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    int16_t tempRaw = (msb << 8) | lsb;
    float temperature = tempRaw * 0.00390625;  // 0.00390625 = (1/256)
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
  delay(100);
}
