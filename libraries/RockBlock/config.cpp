#include "config.h"

// Definiciones de pines
const int STATUS_LED_PIN = 12;
const int KEYPAD_PINS[] = {2, 4, 5};        // Botones 0, 1, 2 (polling)
const int INTERRUPT_BUTTON_PIN = 3;         // Botón 3 (interrupción)

// Definiciones de temporizadores
const unsigned long BEACON_INTERVAL = 10;      // Intervalo para beacons automáticos (segundos)
const int MAX_IRIDIUM_MSG_SENT = 1;            // Límite de beacons automáticos a enviar
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
