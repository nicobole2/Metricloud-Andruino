#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>
#include "config.h"

// =================================================
// --- GPS MANAGER ---
// =================================================
extern TinyGPSPlus gps;

// Funciones públicas
void initGPS();
void handleGPS();
bool isGPSValid();
double getLatitude();
double getLongitude();
double getSpeedKmh();

#endif // GPS_MANAGER_H
