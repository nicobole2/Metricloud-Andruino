#include "config.h"
#include <Arduino.h>

// Definiciones de pines
const int STATUS_LED_PIN = 12;
const int KEYPAD_PINS[] = {2, 4, 5};        // Botones 0, 1, 2 (polling)
const int INTERRUPT_BUTTON_PIN = 3;         // Botón 3 (interrupción)

// Definiciones de temporizadores
unsigned long BEACON_INTERVAL = 10;           // Intervalo para beacons automáticos (segundos) - configurable
int MAX_IRIDIUM_MSG_SENT = 1;                  // Límite de beacons automáticos a enviar (configurable)
const unsigned long STATUS_UPDATE_INTERVAL = 30000UL; // Intervalo para informes de estado (ms)
const unsigned long TEMP_INTERVAL = 3000;      // Intervalo para leer temperaturas (ms)
const unsigned long DEBOUNCE_DELAY = 50;       // Delay para el anti-rebote de los botones (ms)
const unsigned long ENVIADO_DURATION = 5000;   // Duración del mensaje "ENVIADO" en pantalla (ms)

// Definición de mensajes predefinidos
const char* predefinedMessages[] = {
  "Falla mecanica motor",
  "Desvio, camino bloqueado", 
  "Conductor en problemas"
};

// Mensaje de emergencia por defecto para botón de interrupción
const char* EMERGENCY_MESSAGE = "EMERGENCIA - Ayuda requerida";

// Función para configurar el máximo de mensajes Iridium
void setMaxIridiumMessages(int maxMessages) {
  if (maxMessages >= 0) {
    MAX_IRIDIUM_MSG_SENT = maxMessages;
    if (maxMessages == 0) {
      Serial.println(F("Modo SOLO MANUAL: beacons automáticos DESHABILITADOS"));
    } else {
      Serial.print(F("Máximo de beacons configurado a: "));
      Serial.println(MAX_IRIDIUM_MSG_SENT);
    }
  } else {
    Serial.println(F("Error: El máximo de beacons debe ser mayor o igual a 0"));
  }
}

// Función para configurar el intervalo de beacons
void setBeaconInterval(unsigned long intervalSeconds) {
  if (intervalSeconds > 0) {
    BEACON_INTERVAL = intervalSeconds;
    Serial.print(F("Intervalo de beacons configurado a: "));
    Serial.print(BEACON_INTERVAL);
    Serial.println(F(" segundos"));
  } else {
    Serial.println(F("Error: El intervalo de beacons debe ser mayor a 0"));
  }
}
