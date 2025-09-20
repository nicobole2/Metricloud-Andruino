#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

#include <Arduino.h>
#include <DS18B20.h>
#include "config.h"

// =================================================
// --- TEMPERATURE MANAGER ---
// =================================================
extern DS18B20 ds;

// Variables de estado
extern float tempC1, tempC2;
extern uint8_t sensor1Address[8], sensor2Address[8];
extern bool sensorsFound;
extern unsigned long lastTempRequest;

// Funciones públicas
void initTemperatureSensors();
void handleTemperatures(unsigned long currentMillis);
float getTempC1();
float getTempC2();

#endif // TEMPERATURE_MANAGER_H
