#include "utils.h"
#include "temperature_manager.h"
#include "ui_manager.h"
#include "iridium_manager.h"
#include <Arduino.h>

// Variables de estado
unsigned long lastStatusUpdateTime = 0;

void blinkStatusLED() {
  digitalWrite(STATUS_LED_PIN, (millis() / 250) % 2 == 1 ? HIGH : LOW);
}

void printAddress(uint8_t address[]) {
  for (uint8_t i = 0; i < 8; i++) {
    if (address[i] < 16) Serial.print("0");
    Serial.print(address[i], HEX);
  }
}

void updateStatusReport(unsigned long currentMillis) {
  if (currentMillis - lastStatusUpdateTime >= STATUS_UPDATE_INTERVAL) {
    lastStatusUpdateTime = currentMillis;
    long remainingSeconds = (lastTransmissionTime + (BEACON_INTERVAL * 1000UL) - currentMillis) / 1000;
    if (remainingSeconds < 0) remainingSeconds = 0;
    if (iridium_msg_sent_count < MAX_IRIDIUM_MSG_SENT) {
      Serial.print(F("[ESTADO] Próximo beacon en aprox: "));
    } else {
      Serial.print(F("[ESTADO] Próxima revisión de buzón en aprox: "));
    }
    Serial.print(remainingSeconds); Serial.println(F(" segundos."));
  }
}

bool ISBDCallback() {
  unsigned long currentMillis = millis();
  
  // Manejar interrupción del botón 3
  handleInterruptButton();
  
  // Tareas regulares durante transmisión Iridium
  handleTemperatures(currentMillis);
  handleButtons(currentMillis);
  updateDisplay(currentMillis);
  blinkStatusLED();
  
  return true;
}

#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c) { 
  Serial.write(c); 
}

void ISBDDiagsCallback(IridiumSBD *device, char c) { 
  Serial.write(c);
}
#endif
