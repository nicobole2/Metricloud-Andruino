#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <Arduino.h>
#include "config.h"

// =================================================
// --- ESTRUCTURA DE DATOS ---
// =================================================
typedef struct {
    int32_t latitude;      // 4 bytes (escalado por 10^7)
    int32_t longitude;     // 4 bytes (escalado por 10^7)
    int16_t temperature1;  // 2 bytes (escalado por 100)
    int16_t temperature2;  // 2 bytes (escalado por 100)
    uint16_t velocity;     // 2 bytes (escalado por 100)
} __attribute__((packed)) sensor_data_t; // Total: 14 bytes

// =================================================
// --- FUNCIONES DE SERIALIZACIÓN ---
// =================================================
void serialize_sensor_data(double lat, double lon, double temp1, double temp2, double vel, uint8_t* buffer);
void deserialize_sensor_data(uint8_t* buffer, double* lat, double* lon, double* temp1, double* temp2, double* vel);

#if DIAGNOSTICS
void print_sensor_debug(uint8_t* buffer);
#endif

#endif // DATA_STRUCTURES_H
