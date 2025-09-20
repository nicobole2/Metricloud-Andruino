#include "data_structures.h"
#include <Arduino.h>

void serialize_sensor_data(double lat, double lon, double temp1, double temp2, double vel, uint8_t* buffer) {
    sensor_data_t data;
    data.latitude = (int32_t)(lat * GPS_SCALE_FACTOR);
    data.longitude = (int32_t)(lon * GPS_SCALE_FACTOR);
    data.temperature1 = (int16_t)((int)temp1 * TEMP_SCALE_FACTOR);
    data.temperature2 = (int16_t)((int)temp2 * TEMP_SCALE_FACTOR);
    data.velocity = (uint16_t)((int)vel * VELOCITY_SCALE_FACTOR);
    memcpy(buffer, &data, sizeof(sensor_data_t));
}

void deserialize_sensor_data(uint8_t* buffer, double* lat, double* lon, double* temp1, double* temp2, double* vel) {
    sensor_data_t data;
    memcpy(&data, buffer, sizeof(sensor_data_t));
    *lat = (double)data.latitude / GPS_SCALE_FACTOR;
    *lon = (double)data.longitude / GPS_SCALE_FACTOR;
    *temp1 = (double)data.temperature1 / TEMP_SCALE_FACTOR;
    *temp2 = (double)data.temperature2 / TEMP_SCALE_FACTOR;
    *vel = (double)data.velocity / VELOCITY_SCALE_FACTOR;
}

#if DIAGNOSTICS
void print_sensor_debug(uint8_t* buffer) {
    Serial.print("Trama HEX: ");
    for(int i = 0; i < 14; i++) {
        if(buffer[i] < 16) Serial.print("0");
        Serial.print(buffer[i], HEX);
    }
    Serial.println();
}
#endif
