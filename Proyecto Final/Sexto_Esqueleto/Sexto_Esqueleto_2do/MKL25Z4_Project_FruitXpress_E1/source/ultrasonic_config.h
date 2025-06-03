#ifndef ULTRASONIC_CONFIG_H
#define ULTRASONIC_CONFIG_H

#include "hal_ultrasonic.h"
#include "hal_bomba.h"

// Configuración completa de cada tanque
typedef struct {
    const Ultrasonic_Config_t sensor;  // Configuración del sensor
    uint8_t bomba_idx;                 // Índice de la bomba asociada (0-2)
    const char* nombre;                // Nombre del tanque
} Tanque_Config_t;

extern const Tanque_Config_t tanques_config[];
extern const uint32_t NUM_TANQUES;

#endif // ULTRASONIC_CONFIG_H
