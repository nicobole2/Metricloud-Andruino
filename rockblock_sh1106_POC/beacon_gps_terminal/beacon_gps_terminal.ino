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
    setMaxIridiumMessages(3);   // 0 = Solo emergencia, sin beacons automáticos
    setBeaconInterval(30);      // Intervalo para revisión de buzón (solo se usa cuando MAX = 0)
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Comandos serie para debug
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'T' || cmd == 't') {
      Serial.println(F("Simulando interrupción por comando serie..."));
      simulateInterrupt();
    } else if (cmd == 'S' || cmd == 's') {
      testInterruptPin();
    } else if (cmd == 'D' || cmd == 'd') {
      debugInterruptStatus();
    }
  }
  
  handleGPS();
  handleTemperatures(currentMillis);
  handleButtons(currentMillis);
  handleInterruptButton();  // Agregar manejo explícito de interrupción
  checkTransmissionTimeout();  // Verificar timeout de transmisión
  updateDisplay(currentMillis);
  blinkStatusLED();
  
  // Solo ejecutar beacons automáticos si MAX_IRIDIUM_MSG_SENT > 0
  if (MAX_IRIDIUM_MSG_SENT > 0 && currentMillis - lastTransmissionTime >= BEACON_INTERVAL * 1000UL) {
    if (iridium_msg_sent_count < MAX_IRIDIUM_MSG_SENT) {
      attemptTransmission();
    } else {
      checkIncomingMessages();
    }
  }
  
  // Si MAX_IRIDIUM_MSG_SENT = 0, solo revisar mensajes ocasionalmente (sin enviar beacons)
  if (MAX_IRIDIUM_MSG_SENT == 0 && currentMillis - lastTransmissionTime >= BEACON_INTERVAL * 1000UL * 10) {
    checkIncomingMessages();
  }
  updateStatusReport(currentMillis);
}