#include "hardware_setup.h"
#include "gps_manager.h"
#include "iridium_manager.h"
#include "temperature_manager.h"
#include "ui_manager.h"
#include <Arduino.h>

void setupHardware() {
  // Puerto serie
  pinMode(STATUS_LED_PIN, OUTPUT);
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("Puerto Serie - OK"));

  // Inicializar módulos
  initDisplay();
  initButtons();
  initTemperatureSensors();
  initGPS();
  initIridium();
  setDisplayState(DISPLAY_NORMAL);
}

void startupLEDPattern() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(150);
    digitalWrite(STATUS_LED_PIN, LOW); delay(150);
  }
  delay(100);
  for (int i = 0; i < 5; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(250);
    digitalWrite(STATUS_LED_PIN, LOW); delay(250);
  }
}
