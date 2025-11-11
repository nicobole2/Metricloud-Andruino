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
extern unsigned long BEACON_INTERVAL;  // Ahora es variable, no const
extern int MAX_IRIDIUM_MSG_SENT;       // Ahora es variable, no const
extern const unsigned long STATUS_UPDATE_INTERVAL;
extern const unsigned long TEMP_INTERVAL;
extern const unsigned long DEBOUNCE_DELAY;
extern const unsigned long ENVIADO_DURATION;

// Mensajes predefinidos (declaración extern)
extern const char* predefinedMessages[];
extern const char* EMERGENCY_MESSAGE;

// Funciones para configurar parámetros
void setMaxIridiumMessages(int maxMessages);
void setBeaconInterval(unsigned long intervalSeconds);

#endif // CONFIG_H
