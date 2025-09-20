#include "ui_manager.h"
#include "temperature_manager.h"
#include "iridium_manager.h"
#include <Arduino.h>

// Objeto display global
Adafruit_SH1106G display(128, 64, &Wire, -1);

// Variables de estado
int buttonStates[3], lastButtonStates[3];
unsigned long lastDebounceTime[3] = {0};
int selectedMessageIndex = 0;
bool messageSelectedByUser = false;
bool showEnviado = false;
unsigned long enviadoTimestamp = 0;

// Variables de interrupción
volatile bool interruptButtonPressed = false;
volatile unsigned long lastInterruptTime = 0;

void initDisplay() {
  if(!display.begin(0x3C, true)) {
    Serial.println(F("Error: Pantalla SH1106")); 
    for(;;);
  }
  display.clearDisplay();
  display.display();
  Serial.println(F("Pantalla OLED - OK"));
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
  } else {
    Serial.print(F("ERROR: Pin "));
    Serial.print(INTERRUPT_BUTTON_PIN);
    Serial.println(F(" no soporta interrupciones!"));
  }
}

void buttonInterruptISR() {
  // ISR simple y rápida - solo setear flag
  interruptButtonPressed = true;
  // Debug: Parpadear LED built-in para confirmar que ISR se ejecuta
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void handleInterruptButton() {
  // Procesar interrupción en el loop principal (no en ISR)
  if (interruptButtonPressed) {
    interruptButtonPressed = false;
    
    Serial.println(F("*** BOTON 3 DE INTERRUPCION PRESIONADO ***"));
    
    // Ejecutar acción si hay mensaje seleccionado
    if (messageSelectedByUser) {
      sendManualMessage();
    } else {
      Serial.println(F("Selecciona un mensaje primero (botones 0, 1 o 2)"));
    }
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

void handleButtons(unsigned long currentMillis) {
  if (showEnviado) return;
  
  for (int i = 0; i < 3; i++) {
    int reading = digitalRead(KEYPAD_PINS[i]);
    if (reading != lastButtonStates[i]) {
      lastDebounceTime[i] = currentMillis;
    }
    if ((currentMillis - lastDebounceTime[i]) > DEBOUNCE_DELAY) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;
        if (buttonStates[i] == LOW) {
          selectedMessageIndex = i;
          messageSelectedByUser = true;
        }
      }
    }
    lastButtonStates[i] = reading;
  }
}

void updateDisplay(unsigned long currentMillis) {
  if (showEnviado && (currentMillis - enviadoTimestamp >= ENVIADO_DURATION)) {
    showEnviado = false;
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  // Sección Superior
  display.setCursor(0, 0); display.print(F("In Msg:"));
  display.setCursor(74, 0); display.print(F("T1:")); display.print(getTempC1(), 1); display.print(F("C"));
  display.setCursor(0, 10); display.println(incomingMessage);

  // Divisor
  display.drawLine(0, 20, 127, 20, SH110X_WHITE);

  // Sección Inferior
  display.setCursor(0, 24); display.print(F("Out Msg:"));
  display.setCursor(74, 24); display.print(F("T2:")); display.print(getTempC2(), 1); display.print(F("C"));

  if (showEnviado) {
    display.setTextSize(2);
    display.setCursor(20, 40);
    display.print(F("ENVIADO"));
  } else if (messageSelectedByUser) {
    display.setTextSize(1);
    display.setCursor(0, 34);
    display.print(">");
    display.setCursor(8, 34);
    // Manejar texto largo para el mensaje 1
    if (selectedMessageIndex == 1) {
        display.print("Desvio, camino");
        display.setCursor(8, 44);
        display.print("bloqueado");
    } else {
        display.print(predefinedMessages[selectedMessageIndex]);
    }
  }

  display.display();
}

void sendManualMessage() {
  Serial.println(F("Enviando mensaje manual..."));

  char tempC1_str[8], tempC2_str[8];
  dtostrf(getTempC1(), 1, 1, tempC1_str);
  dtostrf(getTempC2(), 1, 1, tempC2_str);

  char outBuffer[120];
  sprintf(outBuffer, "%s | T1:%s, T2:%s", predefinedMessages[selectedMessageIndex], tempC1_str, tempC2_str);
  
  #if DIAGNOSTICS
  Serial.print(F("Mensaje: ")); Serial.println(outBuffer);
  #endif

  showEnviado = true;
  enviadoTimestamp = millis();
  messageSelectedByUser = false;
}
