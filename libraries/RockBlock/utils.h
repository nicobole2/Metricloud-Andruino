#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <IridiumSBD.h>
#include "config.h"

// =================================================
// --- UTILITIES ---
// =================================================

// Variables de estado
extern unsigned long lastStatusUpdateTime;

// Funciones públicas
void blinkStatusLED();
void printAddress(uint8_t address[]);
void updateStatusReport(unsigned long currentMillis);
bool ISBDCallback();

#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c);
void ISBDDiagsCallback(IridiumSBD *device, char c);
#endif

#endif // UTILS_H
