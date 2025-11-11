#include "iridium_manager.h"
#include "gps_manager.h"
#include "temperature_manager.h"
#include "data_structures.h"
#include "ui_manager.h"
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
  setDisplayState(DISPLAY_INITIALIZING, "Iniciando Iridium");
  
  IridiumSerial.begin(19200);
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);
  
  // Los callbacks ISBDConsoleCallback y ISBDDiagsCallback están implementados en utils.cpp
  
  Serial.print(F("Iridium... "));
  int err = modem.begin();
  if (err != ISBD_SUCCESS) {
    Serial.print(F("ERROR: ")); Serial.println(err);
    setDisplayState(DISPLAY_ERROR, "Error Iridium");
    while (true);
  }
  Serial.println(F("OK"));
  setDisplayState(DISPLAY_INITIALIZING, "Iridium OK");
}

void prepareRxBuffer() {
  rxBufferSize = sizeof(rxBuffer);
  memset(rxBuffer, 0, rxBufferSize);
}

void attemptTransmission() {
  lastTransmissionTime = millis();
  Serial.println(F("Transmitiendo beacon..."));
  startTransmission();  // Marcar inicio de transmisión
  setDisplayState(DISPLAY_SENDING);
  
  if (!isGPSValid()) {
    Serial.println(F("GPS no válido"));
    endTransmission();  // Terminar transmisión
    setDisplayState(DISPLAY_ERROR, "Error: GPS no valido");
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
    endTransmission();  // Terminar transmisión
    
    // Verificar si fue cancelación por nueva emergencia
    if (err == ISBD_CANCELLED) {
      Serial.println(F("Beacon cancelado - procesando nueva emergencia"));
      // La nueva emergencia se procesará en el próximo loop
      // No mostrar error, solo permitir que continúe
      return;
    } else {
      Serial.print(F("Error transmisión: ")); Serial.println(err);
      char errorMsg[32];
      sprintf(errorMsg, "Error de transmision: %d", err);
      setDisplayState(DISPLAY_ERROR, errorMsg);
    }
  } else {
    Serial.println(F("Transmisión OK"));
    endTransmission();  // Terminar transmisión
    setDisplayState(DISPLAY_MESSAGE_SENT);
    iridium_msg_sent_count++;
    processReceivedMessage();
  }
}

void checkIncomingMessages() {
  lastTransmissionTime = millis();
  Serial.println(F("Revisando mensajes..."));
  setDisplayState(DISPLAY_RECEIVING);

  prepareRxBuffer();
  if (modem.getWaitingMessageCount() > 0) {
    int err = modem.sendReceiveSBDText(NULL, rxBuffer, rxBufferSize);
    if (err != ISBD_SUCCESS) {
      Serial.print(F("Error buzón: ")); Serial.println(err);
      char errorMsg[32];
      sprintf(errorMsg, "Error de buzon: %d", err);
      setDisplayState(DISPLAY_ERROR, errorMsg);
    } else {
      Serial.println(F("Buzón OK"));
      setDisplayState(DISPLAY_MESSAGE_RECEIVED);
      processReceivedMessage();
    }
  } else {
    Serial.println(F("Sin mensajes"));
    setDisplayState(DISPLAY_NORMAL);
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

void sendTextMessage(const char* message) {
  lastTransmissionTime = millis();
  Serial.println(F("Enviando mensaje de texto..."));
  startTransmission();  // Marcar inicio de transmisión
  
  #if DIAGNOSTICS
  Serial.print(F("Contenido: ")); Serial.println(message);
  #endif
  
  prepareRxBuffer();
  int err = modem.sendReceiveSBDText(message, rxBuffer, rxBufferSize);
  
  if (err != ISBD_SUCCESS) {
    endTransmission();  // Terminar transmisión
    
    // Verificar si fue cancelación por nueva emergencia
    if (err == ISBD_CANCELLED) {
      Serial.println(F("Mensaje de texto cancelado - procesando nueva emergencia"));
      return;
    } else {
      Serial.print(F("Error envío texto: ")); Serial.println(err);
      char errorMsg[32];
      sprintf(errorMsg, "Error de transmision: %d", err);
      setDisplayState(DISPLAY_ERROR, errorMsg);
    }
  } else {
    Serial.println(F("Mensaje de texto enviado OK"));
    endTransmission();  // Terminar transmisión
    setDisplayState(DISPLAY_MESSAGE_SENT);
    // Incrementar contador para mensajes de texto también
    iridium_msg_sent_count++;
    processReceivedMessage();
  }
}
