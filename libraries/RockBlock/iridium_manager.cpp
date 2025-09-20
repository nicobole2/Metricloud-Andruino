#include "iridium_manager.h"
#include "gps_manager.h"
#include "temperature_manager.h"
#include "data_structures.h"
#include <Arduino.h>

// Objeto Iridium global
IridiumSBD modem(IridiumSerial);

// Variables de estado
uint8_t rxBuffer[270];
size_t rxBufferSize;
int iridium_msg_sent_count = 0;
unsigned long lastTransmissionTime = 0;
char incomingMessage[64] = "No new messages";

void initIridium() {
  IridiumSerial.begin(19200);
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);
  
  // Los callbacks ISBDConsoleCallback y ISBDDiagsCallback están implementados en utils.cpp
  
  Serial.print(F("Iridium... "));
  int err = modem.begin();
  if (err != ISBD_SUCCESS) {
    Serial.print(F("ERROR: ")); Serial.println(err);
    while (true);
  }
  Serial.println(F("OK"));
}

void prepareRxBuffer() {
  rxBufferSize = sizeof(rxBuffer);
  memset(rxBuffer, 0, rxBufferSize);
}

void attemptTransmission() {
  lastTransmissionTime = millis();
  Serial.println(F("Transmitiendo beacon..."));
  
  if (!isGPSValid()) {
    Serial.println(F("GPS no válido"));
    return;
  }

  double latitude = getLatitude();
  double longitude = getLongitude();
  double speed_kmh = getSpeedKmh();
  
  #if DIAGNOSTICS
  Serial.print(F("GPS: ")); Serial.print(latitude, 6); 
  Serial.print(F(", ")); Serial.print(longitude, 6);
  Serial.print(F(" | Vel: ")); Serial.print(speed_kmh);
  Serial.print(F(" | T1: ")); Serial.print(getTempC1());
  Serial.print(F(" | T2: ")); Serial.println(getTempC2());
  #endif
  
  uint8_t serial_buffer[14];
  serialize_sensor_data(latitude, longitude, getTempC1(), getTempC2(), speed_kmh, serial_buffer);
  
  #if DIAGNOSTICS
  print_sensor_debug(serial_buffer);
  #endif
  
  prepareRxBuffer();
  int err = modem.sendSBDBinary(serial_buffer, sizeof(serial_buffer));
  
  if (err != ISBD_SUCCESS) {
    Serial.print(F("Error transmisión: ")); Serial.println(err);
  } else {
    Serial.println(F("Transmisión OK"));
    iridium_msg_sent_count++;
    processReceivedMessage();
  }
}

void checkIncomingMessages() {
  lastTransmissionTime = millis();
  Serial.println(F("Revisando mensajes..."));

  prepareRxBuffer();
  if (modem.getWaitingMessageCount() > 0) {
    int err = modem.sendReceiveSBDText(NULL, rxBuffer, rxBufferSize);
    if (err != ISBD_SUCCESS) {
      Serial.print(F("Error buzón: ")); Serial.println(err);
    } else {
      Serial.println(F("Buzón OK"));
      processReceivedMessage();
    }
  } else {
    Serial.println(F("Sin mensajes"));
  }
}

void processReceivedMessage() {
  if (rxBufferSize > 0) {
    strncpy(incomingMessage, (char*)rxBuffer, min(rxBufferSize, sizeof(incomingMessage)-1));
    incomingMessage[min(rxBufferSize, sizeof(incomingMessage)-1)] = '\0';
    Serial.print(F("Mensaje recibido: ")); Serial.println(incomingMessage);
  } else {
    Serial.println(F("Sin mensajes"));
  }
}
