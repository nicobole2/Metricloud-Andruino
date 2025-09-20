#include "temperature_manager.h"
#include <Arduino.h>

// Objeto DS18B20 global
DS18B20 ds(ONE_WIRE_BUS);

// Variables de estado
float tempC1 = -99.0, tempC2 = -99.0;
uint8_t sensor1Address[8], sensor2Address[8];
bool sensorsFound = false;
unsigned long lastTempRequest = 0;

void initTemperatureSensors() {
  Serial.print(F("Sensores temperatura... "));
  if (ds.getNumberOfDevices() >= 2) {
    sensorsFound = true;
    ds.selectNext(); ds.getAddress(sensor1Address);
    ds.selectNext(); ds.getAddress(sensor2Address);
    Serial.println(F("OK"));
  } else {
    Serial.println(F("ERROR"));
  }
}

void handleTemperatures(unsigned long currentMillis) {
  if (currentMillis - lastTempRequest >= TEMP_INTERVAL) {
    lastTempRequest = currentMillis;
    if (sensorsFound) {
      ds.select(sensor1Address);
      float temp1 = ds.getTempC();
      if (temp1 != -127.00) tempC1 = temp1;
      ds.select(sensor2Address);
      float temp2 = ds.getTempC();
      if (temp2 != -127.00) tempC2 = temp2;
    }
  }
}

float getTempC1() {
  return tempC1;
}

float getTempC2() {
  return tempC2;
}
