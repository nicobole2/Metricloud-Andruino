#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"

// =================================================
// --- UI MANAGER ---
// =================================================
extern Adafruit_SH1106G display;

// Variables de estado
extern int buttonStates[3], lastButtonStates[3];
extern unsigned long lastDebounceTime[3];
extern int selectedMessageIndex;
extern bool messageSelectedByUser;
extern bool showEnviado;
extern unsigned long enviadoTimestamp;

// Variables de interrupción
extern volatile bool interruptButtonPressed;
extern volatile unsigned long lastInterruptTime;

// Funciones públicas
void initDisplay();
void initButtons();
void handleButtons(unsigned long currentMillis);
void updateDisplay(unsigned long currentMillis);
void sendManualMessage();
void buttonInterruptISR();
void handleInterruptButton();
void testInterruptPin();
void simulateInterrupt();

#endif // UI_MANAGER_H
