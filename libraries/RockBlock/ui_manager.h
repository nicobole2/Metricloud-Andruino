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

// Estados del display
enum DisplayState {
  DISPLAY_NORMAL,
  DISPLAY_INITIALIZING,
  DISPLAY_SENDING,
  DISPLAY_MESSAGE_SENT,
  DISPLAY_RECEIVING,
  DISPLAY_MESSAGE_RECEIVED,
  DISPLAY_ERROR,
  DISPLAY_EMERGENCY_SENDING
};

// Variables de estado
extern int buttonStates[3], lastButtonStates[3];
extern unsigned long lastDebounceTime[3];
extern int selectedMessageIndex;
extern bool messageSelectedByUser;
extern DisplayState currentDisplayState;
extern unsigned long stateTimestamp;
extern char statusMessage[64];

// Variables de interrupción
extern volatile bool interruptButtonPressed;
extern volatile unsigned long lastInterruptTime;

// Estado global de transmisión
extern bool isTransmitting;
extern unsigned long transmissionStartTime;
extern bool cancelCurrentTransmission;

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
void debugInterruptStatus();
void setDisplayState(DisplayState state, const char* message = "");
void sendEmergencyBeacon();
void startTransmission();
void endTransmission();
void checkTransmissionTimeout();

#endif // UI_MANAGER_H
