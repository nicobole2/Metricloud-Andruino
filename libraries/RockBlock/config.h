#ifndef CONFIG_H
#define CONFIG_H

// =================================================
// --- CONFIGURACIÓN DE HARDWARE ---
// =================================================
#define IridiumSerial        Serial3
#define GPSSerial            Serial1
#define GPSBaud              9600
#define ONE_WIRE_BUS         7
#define DIAGNOSTICS          true

// Pines (declaraciones extern)
extern const int STATUS_LED_PIN;
extern const int KEYPAD_PINS[];
extern const int INTERRUPT_BUTTON_PIN;

// Factores de escalado
#define GPS_SCALE_FACTOR     10000000       // 10^7 para coordenadas
#define TEMP_SCALE_FACTOR    100            // 10^2 para temperaturas  
#define VELOCITY_SCALE_FACTOR 100           // 10^2 para velocidad

// Temporizadores (declaraciones extern)
extern const unsigned long BEACON_INTERVAL;
extern const int MAX_IRIDIUM_MSG_SENT;
extern const unsigned long STATUS_UPDATE_INTERVAL;
extern const unsigned long TEMP_INTERVAL;
extern const unsigned long DEBOUNCE_DELAY;
extern const unsigned long ENVIADO_DURATION;

// Mensajes predefinidos (declaración extern)
extern const char* predefinedMessages[];

#endif // CONFIG_H
