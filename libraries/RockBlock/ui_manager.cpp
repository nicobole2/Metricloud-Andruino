#include "ui_manager.h"
#include "temperature_manager.h"
#include "iridium_manager.h"
#include "gps_manager.h"
#include "data_structures.h"
#include <Arduino.h>

// Objeto display global
Adafruit_SH1106G display(128, 64, &Wire, -1);

// Variables de estado
int buttonStates[3], lastButtonStates[3];
unsigned long lastDebounceTime[3] = {0};
int selectedMessageIndex = 0;
bool messageSelectedByUser = false;
DisplayState currentDisplayState = DISPLAY_NORMAL;
unsigned long stateTimestamp = 0;
char statusMessage[64] = "";

// Variables de interrupción
volatile bool interruptButtonPressed = false;
volatile unsigned long lastInterruptTime = 0;

// Estado global de transmisión
bool isTransmitting = false;
unsigned long transmissionStartTime = 0;
bool cancelCurrentTransmission = false;

void initDisplay() {
  if(!display.begin(0x3C, true)) {
    Serial.println(F("Error: Pantalla SH1106")); 
    for(;;);
  }
  display.clearDisplay();
  display.display();
  Serial.println(F("Pantalla OLED - OK"));
  
  // Mostrar estado de inicialización
  setDisplayState(DISPLAY_INITIALIZING, "Pantalla OK");
}

void initButtons() {
  // Configurar LED built-in para debug
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  for (int i = 0; i < 3; i++) {
    pinMode(KEYPAD_PINS[i], INPUT_PULLUP);
    buttonStates[i] = lastButtonStates[i] = HIGH;
  }
  
  // Configurar botón de interrupción (PIN 3)
  pinMode(INTERRUPT_BUTTON_PIN, INPUT_PULLUP);
  
  // Verificar si el pin soporta interrupciones
  int interruptNum = digitalPinToInterrupt(INTERRUPT_BUTTON_PIN);
  if (interruptNum != NOT_AN_INTERRUPT) {
    attachInterrupt(interruptNum, buttonInterruptISR, FALLING);
    Serial.print(F("Botones - OK (Interrupción en pin "));
    Serial.print(INTERRUPT_BUTTON_PIN);
    Serial.print(F(" = INT"));
    Serial.print(interruptNum);
    Serial.println(F(")"));
    
    // Diagnóstico adicional
    Serial.print(F("Estado inicial del pin: ")); 
    Serial.println(digitalRead(INTERRUPT_BUTTON_PIN) ? "HIGH" : "LOW");
    Serial.println(F("Interrupción configurada correctamente"));
  } else {
    Serial.print(F("ERROR: Pin "));
    Serial.print(INTERRUPT_BUTTON_PIN);
    Serial.println(F(" no soporta interrupciones!"));
  }
}

void buttonInterruptISR() {
  // ISR extremadamente simple - SOLO setear flag
  // NO hacer millis() u otras operaciones en ISR
  interruptButtonPressed = true;
}

void handleInterruptButton() {
  // Procesar interrupción en el loop principal (no en ISR)
  if (interruptButtonPressed) {
    Serial.println(F("DEBUG: Flag de interrupción detectado"));
    
    // Si hay transmisión en curso, marcar para cancelar
    if (isTransmitting) {
      Serial.println(F("*** CANCELANDO TRANSMISIÓN ACTUAL - NUEVA EMERGENCIA ***"));
      cancelCurrentTransmission = true;
      // NO reset del flag aquí - se necesita para el reintento
      // interruptButtonPressed permanece true para procesar después
      return;
    }
    
    interruptButtonPressed = false;  // Reset flag inmediatamente
    
    Serial.println(F("*** BOTON DE EMERGENCIA PRESIONADO ***"));
    Serial.print(F("Pin state: ")); 
    Serial.println(digitalRead(INTERRUPT_BUTTON_PIN) ? "HIGH" : "LOW");
    
    // Parpadear LED para confirmar recepción
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    
    // Enviar beacon de emergencia inmediatamente
    sendEmergencyBeacon();
  }
}

void testInterruptPin() {
  Serial.println(F("=== TEST DE INTERRUPCION PIN 3 ==="));
  Serial.print(F("INTERRUPT_BUTTON_PIN = ")); Serial.println(INTERRUPT_BUTTON_PIN);
  Serial.print(F("digitalPinToInterrupt(3) = ")); Serial.println(digitalPinToInterrupt(3));
  Serial.print(F("Estado actual del pin 3: ")); Serial.println(digitalRead(INTERRUPT_BUTTON_PIN) ? "HIGH" : "LOW");
  Serial.println(F("Presiona el botón 3 para probar..."));
  Serial.println(F("O envía 'T' por Serial para simular interrupción"));
  Serial.println(F("====================================="));
}

void simulateInterrupt() {
  Serial.println(F("SIMULANDO INTERRUPCION..."));
  interruptButtonPressed = true;
  digitalWrite(LED_BUILTIN, HIGH);
}

void debugInterruptStatus() {
  // Debug manual solamente - no automático
  Serial.println(F("=== DEBUG INTERRUPCION ==="));
  Serial.print(F("Pin 3 estado: ")); 
  Serial.println(digitalRead(INTERRUPT_BUTTON_PIN) ? "HIGH" : "LOW");
  Serial.print(F("Flag interrupción: ")); 
  Serial.println(interruptButtonPressed ? "TRUE" : "FALSE");
  Serial.print(F("Última interrupción: ")); 
  Serial.print(lastInterruptTime); Serial.println(F(" ms"));
  Serial.print(F("Tiempo actual: ")); 
  Serial.print(millis()); Serial.println(F(" ms"));
  Serial.println(F("========================"));
}

void setDisplayState(DisplayState state, const char* message) {
  currentDisplayState = state;
  stateTimestamp = millis();
  
  if (message && strlen(message) > 0) {
    strncpy(statusMessage, message, sizeof(statusMessage) - 1);
    statusMessage[sizeof(statusMessage) - 1] = '\0';
  } else {
    statusMessage[0] = '\0';
  }
}

void handleButtons(unsigned long currentMillis) {
  // Los botones 0, 1, 2 ya no se usan para seleccionar mensajes
  // Solo el botón 3 (interrupción) se usa para emergencias
  // Esta función se mantiene para compatibilidad pero no hace nada
  (void)currentMillis; // Evitar warning de variable no usada
}

void updateDisplay(unsigned long currentMillis) {
  // Auto-retornar a estado normal después de un tiempo
  if (currentDisplayState != DISPLAY_NORMAL && 
      currentDisplayState != DISPLAY_INITIALIZING &&
      (currentMillis - stateTimestamp >= ENVIADO_DURATION)) {
    setDisplayState(DISPLAY_NORMAL);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  
  // Mostrar estado principal en el centro
  switch (currentDisplayState) {
    case DISPLAY_INITIALIZING:
      display.setTextSize(1);
      display.setCursor(30, 20);
      display.print(F("Iniciando..."));
      if (strlen(statusMessage) > 0) {
        display.setCursor(10, 35);
        display.print(statusMessage);
      }
      break;
      
    case DISPLAY_SENDING:
      display.setTextSize(1);
      display.setCursor(30, 28);
      display.print(F("Mandando..."));
      break;
      
    case DISPLAY_MESSAGE_SENT:
      display.setTextSize(2);
      display.setCursor(10, 25);
      display.print(F("Mensaje"));
      display.setCursor(20, 45);
      display.print(F("enviado"));
      break;
      
    case DISPLAY_RECEIVING:
      display.setTextSize(1);
      display.setCursor(25, 28);
      display.print(F("Recibiendo..."));
      break;
      
    case DISPLAY_MESSAGE_RECEIVED:
      display.setTextSize(1);
      display.setCursor(20, 20);
      display.print(F("Mensaje"));
      display.setCursor(20, 35);
      display.print(F("recibido"));
      break;
      
    case DISPLAY_ERROR:
      // El error se muestra en el display normal, en la línea inferior
      // Usar el mismo layout que DISPLAY_NORMAL pero con mensaje de error abajo
      display.setTextSize(1);
      
      // Línea superior: Temperaturas
      display.setCursor(0, 0);
      display.print(F("T1:")); display.print(getTempC1(), 1); display.print(F("C"));
      display.setCursor(64, 0);
      display.print(F("T2:")); display.print(getTempC2(), 1); display.print(F("C"));
      
      // Línea de separación
      display.drawLine(0, 12, 127, 12, SH110X_WHITE);
      
      // Estado del sistema
      display.setCursor(0, 20);
      display.print(F("Beacons enviados: "));
      display.print(iridium_msg_sent_count);
      
      display.setCursor(0, 35);
      if (MAX_IRIDIUM_MSG_SENT == 0) {
        display.print(F("Modo: SOLO MANUAL"));
      } else {
        display.print(F("Modo: AUTOMATICO"));
      }
      
      // El mensaje de error se maneja en la lógica de abajo
      break;
      
    case DISPLAY_EMERGENCY_SENDING:
      // Mostrar interfaz normal pero con estado de emergencia
      display.setTextSize(1);
      
      // Línea superior: Temperaturas
      display.setCursor(0, 0);
      display.print(F("T1:")); display.print(getTempC1(), 1); display.print(F("C"));
      display.setCursor(64, 0);
      display.print(F("T2:")); display.print(getTempC2(), 1); display.print(F("C"));
      
      // Línea de separación
      display.drawLine(0, 12, 127, 12, SH110X_WHITE);
      
      // Estado del sistema
      display.setCursor(0, 20);
      display.print(F("Beacons enviados: "));
      display.print(iridium_msg_sent_count);
      
      display.setCursor(0, 35);
      if (MAX_IRIDIUM_MSG_SENT == 0) {
        display.print(F("Modo: SOLO MANUAL"));
      } else {
        display.print(F("Modo: AUTOMATICO"));
      }
      
      // Mostrar estado de envío
      display.setCursor(0, 50);
      display.print(F("Mandando mensaje..."));
      break;
      
    case DISPLAY_NORMAL:
    default:
      // Mostrar interfaz normal
      display.setTextSize(1);
      
      // Línea superior: Temperaturas
      display.setCursor(0, 0);
      display.print(F("T1:")); display.print(getTempC1(), 1); display.print(F("C"));
      display.setCursor(64, 0);
      display.print(F("T2:")); display.print(getTempC2(), 1); display.print(F("C"));
      
      // Línea de separación
      display.drawLine(0, 12, 127, 12, SH110X_WHITE);
      
      // Estado del sistema
      display.setCursor(0, 20);
      display.print(F("Beacons enviados: "));
      display.print(iridium_msg_sent_count);
      
      display.setCursor(0, 35);
      if (MAX_IRIDIUM_MSG_SENT == 0) {
        display.print(F("Modo: SOLO MANUAL"));
      } else {
        display.print(F("Modo: AUTOMATICO"));
      }
      
      // Mostrar estado de envío/recepción/error en la línea inferior
      display.setCursor(0, 50);
      if (isTransmitting) {
        display.print(F("Mandando mensaje..."));
      } else if (currentDisplayState == DISPLAY_RECEIVING) {
        display.print(F("Revisando buzon..."));
      } else if (currentDisplayState == DISPLAY_ERROR) {
        // Mostrar mensaje de error en la misma línea
        if (strlen(statusMessage) > 0) {
          display.print(statusMessage);
        } else {
          display.print(F("Error transmision"));
        }
      } else {
        // Línea vacía cuando no está transmitiendo
        display.print(F("                "));  // Limpiar línea
      }
      break;
  }

  display.display();
}

void sendManualMessage() {
  Serial.println(F("Preparando mensaje manual..."));

  // Verificar GPS válido
  if (!isGPSValid()) {
    Serial.println(F("GPS no válido - no se puede enviar mensaje"));
    setDisplayState(DISPLAY_ERROR, "GPS no valido");
    return;
  }

  // Mostrar estado de envío
  setDisplayState(DISPLAY_SENDING);

  // Obtener datos GPS
  double latitude = getLatitude();
  double longitude = getLongitude();
  double speed_kmh = getSpeedKmh();

  // Obtener temperaturas
  char tempC1_str[8], tempC2_str[8];
  dtostrf(getTempC1(), 1, 1, tempC1_str);
  dtostrf(getTempC2(), 1, 1, tempC2_str);

  // Crear mensaje con coordenadas, temperaturas y velocidad
  char outBuffer[200];
  sprintf(outBuffer, "%s | GPS:%.6f,%.6f | Vel:%.1fkm/h | T1:%s, T2:%s", 
          predefinedMessages[selectedMessageIndex], 
          latitude, longitude, speed_kmh,
          tempC1_str, tempC2_str);
  
  // Enviar el mensaje usando el Iridium
  sendTextMessage(outBuffer);

  messageSelectedByUser = false;
}

void startTransmission() {
  // PROTECCIÓN: No permitir múltiples transmisiones simultáneas
  if (isTransmitting) {
    Serial.println(F("DEBUG: Ya hay transmisión en curso"));
    return;
  }
  
  isTransmitting = true;
  transmissionStartTime = millis();
  cancelCurrentTransmission = false;  // Reset cancelación
  // Limpiar cualquier estado de error previo
  if (currentDisplayState == DISPLAY_ERROR) {
    setDisplayState(DISPLAY_NORMAL);
  }
  Serial.println(F("DEBUG: Iniciando transmisión"));
}

void endTransmission() {
  isTransmitting = false;
  Serial.print(F("DEBUG: Transmisión terminada después de "));
  Serial.print(millis() - transmissionStartTime);
  Serial.println(F(" ms"));
}

void checkTransmissionTimeout() {
  // Timeout de seguridad: si lleva más de 60 segundos transmitiendo, algo está mal
  if (isTransmitting && (millis() - transmissionStartTime > 60000)) {
    Serial.println(F("DEBUG: Timeout de transmisión detectado, forzando fin"));
    endTransmission();
    setDisplayState(DISPLAY_ERROR, "Error: timeout");
  }
}

void sendEmergencyBeacon() {
  Serial.println(F("*** ENVIANDO BEACON DE EMERGENCIA ***"));
  startTransmission();  // Marcar inicio de transmisión
  setDisplayState(DISPLAY_EMERGENCY_SENDING, "EMERGENCIA!");
  
  // Para emergencia, enviar siempre, incluso sin GPS válido
  double latitude = 0.0, longitude = 0.0, speed_kmh = 0.0;
  
  if (isGPSValid()) {
    latitude = getLatitude();
    longitude = getLongitude(); 
    speed_kmh = getSpeedKmh();
    Serial.println(F("GPS válido - incluyendo coordenadas"));
  } else {
    Serial.println(F("GPS no válido - enviando con coordenadas 0,0"));
  }
  
  // Obtener temperaturas (siempre disponibles)
  double temp1 = getTempC1();
  double temp2 = getTempC2();
  
  #if DIAGNOSTICS
  Serial.print(F("EMERGENCIA GPS: ")); Serial.print(latitude, 6); 
  Serial.print(F(", ")); Serial.print(longitude, 6);
  Serial.print(F(" | Vel: ")); Serial.print(speed_kmh);
  Serial.print(F(" | T1: ")); Serial.print(temp1);
  Serial.print(F(" | T2: ")); Serial.println(temp2);
  #endif
  
  // Crear buffer binario con marcador de emergencia
  uint8_t emergency_buffer[14];
  serialize_sensor_data(latitude, longitude, temp1, temp2, speed_kmh, emergency_buffer);
  
  // Modificar el primer byte para marcar como emergencia (0xFF)
  emergency_buffer[0] = 0xFF;
  
  #if DIAGNOSTICS
  Serial.print(F("EMERGENCIA - Trama HEX: "));
  for(int i = 0; i < 14; i++) {
    if(emergency_buffer[i] < 16) Serial.print("0");
    Serial.print(emergency_buffer[i], HEX);
  }
  Serial.println();
  #endif
  
  // Enviar usando formato binario
  prepareRxBuffer();
  int err = modem.sendSBDBinary(emergency_buffer, sizeof(emergency_buffer));
  
  if (err != ISBD_SUCCESS) {
    endTransmission();  // Terminar transmisión
    
    // Verificar si fue cancelación por nueva emergencia
    if (err == ISBD_CANCELLED) {
      Serial.println(F("Emergencia cancelada - procesando nueva emergencia"));
      // La nueva emergencia se procesará en el próximo loop
      // No mostrar error, solo reintentar
      return;
    } else {
      Serial.print(F("Error emergencia: ")); Serial.println(err);
      char errorMsg[32];
      sprintf(errorMsg, "Error de transmision: %d", err);
      setDisplayState(DISPLAY_ERROR, errorMsg);
    }
  } else {
    Serial.println(F("*** EMERGENCIA ENVIADA OK ***"));
    endTransmission();  // Terminar transmisión
    setDisplayState(DISPLAY_MESSAGE_SENT, "Emergencia enviada");
    // Incrementar contador para emergencias también
    iridium_msg_sent_count++;
    processReceivedMessage();
  }
}
