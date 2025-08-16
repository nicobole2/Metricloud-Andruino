
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

// =================================================
// --- LIBRERÍAS ---
// =================================================
#include <IridiumSBD.h>
#include <TinyGPS++.h>
#include <DS18B20.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h> // LIBRERÍA MODIFICADA PARA SH1106

// =================================================
// --- CONFIGURACIÓN DE PINES Y PUERTOS SERIE ---
// =================================================
#define IridiumSerial        Serial3       // Puerto para el Módem Iridium
#define GPSSerial            Serial1       // Puerto para el Módulo GPS
#define GPSBaud              9600          // Baud rate del GPS
#define ONE_WIRE_BUS         7             // Pin para los sensores de temperatura DS18B20
const int STATUS_LED_PIN =   12;           // Pin para el LED de estado
const int KEYPAD_PINS[] =    {2, 3, 4, 5}; // Pines para botones 1, 2, 3, 4

// =================================================
// --- AJUSTES DE LÓGICA Y TEMPORIZADORES ---
// =================================================
#define DIAGNOSTICS true
const unsigned long BEACON_INTERVAL = 65;      // Intervalo para beacons automáticos (segundos)
const int MAX_IRIDIUM_MSG_SENT = 3;            // Límite de beacons automáticos a enviar
const unsigned long STATUS_UPDATE_INTERVAL = 30000UL; // Intervalo para informes de estado (ms)
const unsigned long TEMP_INTERVAL = 3000;      // Intervalo para leer temperaturas (ms)
const unsigned long DEBOUNCE_DELAY = 50;       // Delay para el anti-rebote de los botones (ms)
const unsigned long ENVIADO_DURATION = 5000;   // Duración del mensaje "ENVIADO" en pantalla (ms)

// =================================================
// --- OBJETOS GLOBALES ---
// =================================================
IridiumSBD modem(IridiumSerial);
TinyGPSPlus gps;
// OBJETO DE PANTALLA MODIFICADO para el controlador SH1106 I2C 128x64
Adafruit_SH1106G display(128, 64, &Wire, -1);
DS18B20 ds(ONE_WIRE_BUS);

// =================================================
// --- VARIABLES DE ESTADO ---
// =================================================
// Estado de Iridium y GPS
uint8_t rxBuffer[270];
size_t rxBufferSize;
int iridium_msg_sent_count = 0;
unsigned long lastTransmissionTime = 0;
unsigned long lastStatusUpdateTime = 0;
// Estado de la Interfaz de Usuario
float tempC1 = -99.0, tempC2 = -99.0;
uint8_t sensor1Address[8], sensor2Address[8];
bool sensorsFound = false;
unsigned long lastTempRequest = 0;
int buttonStates[4], lastButtonStates[4];
unsigned long lastDebounceTime[4] = {0};
String incomingMessage = "No new messages";
String predefinedMessages[] = {
  "Falla mecanica motor",
  "Desvio, camino bloqueado",
  "Conductor en problemas"
};
int selectedMessageIndex = 0;
bool messageSelectedByUser = false;
bool showEnviado = false;
unsigned long enviadoTimestamp = 0;

// =================================================
// --- SETUP ---
// =================================================
void setup() {
  setupHardware();
  startupLEDPattern();
  Serial.println(F(">>> Sistema Integrado V5 (SH1106) Inicializado. <<<"));
}

// =================================================
// --- LOOP ---
// =================================================
unsigned long currentMillis = 0;

void loop() {
  currentMillis = millis();

  // --- Tareas de Alta Frecuencia (ejecutadas en cada ciclo) ---
  handleGPS();
  handleTemperatures(currentMillis);
  handleButtons(currentMillis);
  updateDisplay(currentMillis);
  blinkStatusLED();
  // --- Tareas de Baja Frecuencia (controladas por tiempo) ---
  if (currentMillis - lastTransmissionTime >= BEACON_INTERVAL * 1000UL) {
    if (iridium_msg_sent_count < MAX_IRIDIUM_MSG_SENT) {
      attemptTransmission();
    } else {
      checkIncomingMessages();
    }
  }
  updateStatusReport(currentMillis);
}

// =================================================
// --- FUNCIONES DE CONFIGURACIÓN ---
// =================================================

void setupHardware() {
  pinMode(STATUS_LED_PIN, OUTPUT);
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("1. Puerto Serie (USB) - OK"));

  // INICIALIZACIÓN MODIFICADA para SH1106
  if(!display.begin(0x3C, true)) { // true = reset controller
    Serial.println(F("Fallo al iniciar SH1106")); for(;;);
  }
  display.clearDisplay();
  display.display();
  Serial.println(F("2. Pantalla OLED - OK"));

  for (int i = 0; i < 4; i++) {
    pinMode(KEYPAD_PINS[i], INPUT_PULLUP);
    buttonStates[i] = HIGH;
    lastButtonStates[i] = HIGH;
  }
  Serial.println(F("3. Teclado - OK"));

  Serial.print(F("4. Buscando Sensores de Temperatura... "));
  if (ds.getNumberOfDevices() >= 2) {
    sensorsFound = true;
    ds.selectNext(); ds.getAddress(sensor1Address);
    ds.selectNext(); ds.getAddress(sensor2Address);
    Serial.println(F("OK (2 encontrados)"));
  } else {
    Serial.println(F("FALLO (se necesitan 2)"));
  }

  GPSSerial.begin(GPSBaud);
  Serial.println(F("5. Puerto GPS - OK"));

  IridiumSerial.begin(19200);
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);
  Serial.print(F("6. Iniciando Módem Iridium... "));
  int err = modem.begin();
  if (err != ISBD_SUCCESS) {
    Serial.print(F("FALLO. Error: "));
    Serial.println(err);
    if (err == ISBD_NO_MODEM_DETECTED) {
      Serial.println(F("--> No se detecta el módem. Revisa las conexiones."));
    }
    while (true);
  }
  Serial.println(F("OK"));
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

// =================================================
// --- FUNCIONES DE LÓGICA PRINCIPAL ---
// =================================================

void handleGPS() {
  while (GPSSerial.available() > 0) {
    gps.encode(GPSSerial.read());
  }
}

void attemptTransmission() {
  lastTransmissionTime = millis();
  Serial.println(F("\n----------------------------------------"));
  Serial.println(F("Intentando transmitir beacon automático..."));
  if (gps.location.isValid() && gps.location.age() < 30000) {
    Serial.println("-> Fix de GPS reciente y válido.");
  } else {
    Serial.println("-> Fallo: No hay un fix de GPS reciente. Saltando este intervalo.");
    return;
  }

  char outBuffer[80];
  sprintf(outBuffer, "%02d%02dT%02d%02d%02d,ID%d,%s%u.%09lu,%s%u.%09lu",
      gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second(),
      iridium_msg_sent_count + 1,
      gps.location.rawLat().negative ? "-" : "", gps.location.rawLat().deg, gps.location.rawLat().billionths,
      gps.location.rawLng().negative ? "-" : "", gps.location.rawLng().deg, gps.location.rawLng().billionths
  );
  Serial.print(F("-> Mensaje a enviar: ")); Serial.println(outBuffer);

  rxBufferSize = sizeof(rxBuffer);
  memset(rxBuffer, 0, rxBufferSize);
  int err = modem.sendReceiveSBDText(outBuffer, rxBuffer, rxBufferSize);
  if (err != ISBD_SUCCESS) {
    Serial.print(F("-> Fallo en la transmisión. Error: ")); Serial.println(err);
  } else {
    Serial.println(F("-> ¡Transmisión exitosa!"));
    iridium_msg_sent_count++;
    processReceivedMessage();
  }
}

void checkIncomingMessages() {
  lastTransmissionTime = millis();
  Serial.println(F("\n----------------------------------------"));
  Serial.println(F("[INFO] Límite de envíos alcanzado. Revisando solo mensajes entrantes..."));

  rxBufferSize = sizeof(rxBuffer);
  memset(rxBuffer, 0, rxBufferSize);
  int err = modem.sendReceiveSBDText(NULL, rxBuffer, rxBufferSize);
  if (err != ISBD_SUCCESS) {
    Serial.print("-> Fallo en la revisión del buzón. Error: ");
    Serial.println(err);
  } else {
    Serial.println(F("-> Revisión de buzón exitosa."));
    processReceivedMessage();
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

// =================================================
// --- FUNCIONES DE INTERFAZ DE USUARIO (UI) ---
// =================================================

void handleTemperatures(unsigned long currentMillis) {
  if (currentMillis - lastTempRequest >= TEMP_INTERVAL) {
    lastTempRequest = currentMillis;
    if (sensorsFound) {
      ds.select(sensor1Address);
      float temp1 = ds.getTempC();
      if (temp1 != -127.00) tempC1 = temp1;
      ds.select(sensor2Address);
      float temp2 = ds.getTempC();
      if (temp2 != -127.00) tempC2 = temp2;
    }
  }
}

void handleButtons(unsigned long currentMillis) {
  if (showEnviado) return;
  for (int i = 0; i < 4; i++) {
    int reading = digitalRead(KEYPAD_PINS[i]);
    if (reading != lastButtonStates[i]) {
      lastDebounceTime[i] = currentMillis;
    }
    if ((currentMillis - lastDebounceTime[i]) > DEBOUNCE_DELAY) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;
        if (buttonStates[i] == LOW) {
          if (i < 3) {
            selectedMessageIndex = i;
            messageSelectedByUser = true;
          } else if (i == 3 && messageSelectedByUser) {
            sendManualMessage();
          }
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
  display.setCursor(74, 0); display.print(F("T1:")); display.print(tempC1, 1); display.print(F("C"));
  display.setCursor(0, 10); display.println(incomingMessage);

  // Divisor
  display.drawLine(0, 20, 127, 20, SH110X_WHITE);

  // Sección Inferior
  display.setCursor(0, 24); display.print(F("Out Msg:"));
  display.setCursor(74, 24); display.print(F("T2:")); display.print(tempC2, 1); display.print(F("C"));


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

// =================================================
// --- FUNCIONES DE COMUNICACIÓN ---
// =================================================

void sendManualMessage() {
  Serial.println(F("\n----------------------------------------"));
  Serial.println(F("Boton de envio presionado. Preparando mensaje manual..."));

// 1. Crear buffers (arrays de char) para guardar las temperaturas como texto.
  // Un tamaño de 8 es seguro para números como "-123.45\0".
  char tempC1_str[8];
  char tempC2_str[8];

  // 2. Convertir los floats a texto usando dtostrf().
  // Formato: dtostrf(variable_float, ancho_minimo, decimales, buffer_destino);
  // Un ancho mínimo de 1 significa que no hay relleno de espacios.
  dtostrf(tempC1, 1, 1, tempC1_str); // 1 decimal de precisión
  dtostrf(tempC2, 1, 1, tempC2_str); // 1 decimal de precisión

  // 3. Usar los buffers de texto en sprintf() con %s en lugar de %.1f
  char outBuffer[120];
  sprintf(outBuffer, "%s | T1:%s, T2:%s", predefinedMessages[selectedMessageIndex].c_str(), tempC1_str, tempC2_str);
  Serial.print("-> Mensaje manual a enviar: "); Serial.println(outBuffer);

  rxBufferSize = sizeof(rxBuffer);
  memset(rxBuffer, 0, rxBufferSize);
  int err = modem.sendReceiveSBDText(outBuffer, rxBuffer, rxBufferSize);
  if (err == ISBD_SUCCESS) {
    Serial.println("-> Mensaje manual enviado con exito.");
    processReceivedMessage();
  } else {
    Serial.print("-> Fallo en envio manual. Error: "); Serial.println(err);
  }

  showEnviado = true;
  enviadoTimestamp = millis();
  messageSelectedByUser = false;
}

void processReceivedMessage() {
  if (rxBufferSize > 0) {
    incomingMessage = "";
    for(size_t i=0; i < rxBufferSize; ++i) {
        incomingMessage += (char)rxBuffer[i];
    }
    Serial.println(F("--- ¡MENSAJE ENTRANTE RECIBIDO! ---"));
    Serial.print(F("  Contenido: ")); Serial.println(incomingMessage);
  } else {
    Serial.println(F("-> No había mensajes esperando."));
  }
}

// =================================================
// --- FUNCIONES DE CALLBACK Y UTILIDAD ---
// =================================================

void blinkStatusLED() {
  digitalWrite(STATUS_LED_PIN, (millis() / 250) % 2 == 1 ? HIGH : LOW);
}

void printAddress(uint8_t address[]) {
  for (uint8_t i = 0; i < 8; i++) {
    if (address[i] < 16) Serial.print("0");
    Serial.print(address[i], HEX);
  }
}

bool ISBDCallback() {
  handleTemperatures(currentMillis);
  handleButtons(currentMillis);
  updateDisplay(currentMillis);
  blinkStatusLED();

  return true;
}

#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c) { Serial.write(c); }
void ISBDDiagsCallback(IridiumSBD *device, char c) { Serial.write(c);
}
#endif
