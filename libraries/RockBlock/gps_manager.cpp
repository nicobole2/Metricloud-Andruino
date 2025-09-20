#include "gps_manager.h"
#include <Arduino.h>

// Objeto GPS global
TinyGPSPlus gps;

void initGPS() {
  GPSSerial.begin(GPSBaud);
  Serial.println(F("GPS - OK"));
}

void handleGPS() {
  while (GPSSerial.available() > 0) {
    gps.encode(GPSSerial.read());
  }
}

bool isGPSValid() {
  return gps.location.isValid() && gps.location.age() < 30000;
}

double getLatitude() {
  return gps.location.lat();
}

double getLongitude() {
  return gps.location.lng();
}

double getSpeedKmh() {
  return gps.speed.kmph();
}
