// =================================================================
//     PROYECTO INTEGRADO V5: BEACON GPS + TERMINAL DE USUARIO SATELITAL (CON SH1106)
// =================================================================
// Autor Original: LMRIOS
// Refactorizado e Integrado por: Gemini
// Fecha de Migración: 30/06/2025
//
// Descripción:
// Versión final que fusiona un beacon GPS satelital con una interfaz de
// usuario completa (OLED, teclado, sensores de temperatura).
// Este código ha sido migrado para usar una pantalla OLED con controlador SH1106.
// El sistema puede enviar beacons automáticos y mensajes manuales definidos por el usuario.
//
// Formato binario (little-endian):
// Bytes 0-3:   Latitud (int32) / 10,000,000 = grados decimales
// Bytes 4-7:   Longitud (int32) / 10,000,000 = grados decimales  
// Bytes 8-9:   Temperatura 1 (int16) / 100 = °C
// Bytes 10-11: Temperatura 2 (int16) / 100 = °C
// Bytes 12-13: Velocidad (uint16) / 100 = km/h

// =================================================
// --- INCLUDES DE MÓDULOS ---
// =================================================
#include "config.h"
#include "data_structures.h"
#include "gps_manager.h"
#include "iridium_manager.h"
#include "temperature_manager.h"
#include "ui_manager.h"
#include "hardware_setup.h"
#include "utils.h"

// =================================================
// --- SETUP Y LOOP PRINCIPAL ---
// =================================================
void setup() {
    setupHardware();
    startupLEDPattern();
}

void loop() {
  unsigned long currentMillis = millis();
  
  handleGPS();
  handleTemperatures(currentMillis);
  handleButtons(currentMillis);
  updateDisplay(currentMillis);
  blinkStatusLED();
  
  if (currentMillis - lastTransmissionTime >= BEACON_INTERVAL * 1000UL) {
    if (iridium_msg_sent_count < MAX_IRIDIUM_MSG_SENT) {
      attemptTransmission();
    } else {
      checkIncomingMessages();
    }
  }
  updateStatusReport(currentMillis);
}