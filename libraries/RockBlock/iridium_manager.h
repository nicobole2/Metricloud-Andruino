#ifndef IRIDIUM_MANAGER_H
#define IRIDIUM_MANAGER_H

#include <Arduino.h>
#include <IridiumSBD.h>
#include "config.h"

// =================================================
// --- IRIDIUM MANAGER ---
// =================================================
extern IridiumSBD modem;

// Variables de estado
extern uint8_t rxBuffer[270];
extern size_t rxBufferSize;
extern int iridium_msg_sent_count;
extern unsigned long lastTransmissionTime;
extern char incomingMessage[64];

// Funciones públicas
void initIridium();
void prepareRxBuffer();
void attemptTransmission();
void checkIncomingMessages();
void processReceivedMessage();
void sendTextMessage(const char* message);

#endif // IRIDIUM_MANAGER_H
