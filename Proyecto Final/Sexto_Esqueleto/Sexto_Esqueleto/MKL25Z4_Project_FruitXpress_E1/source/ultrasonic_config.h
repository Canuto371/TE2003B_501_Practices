#ifndef ULTRASONIC_CONFIG_H
#define ULTRASONIC_CONFIG_H

#include "hal_ultrasonic.h"
#include "hal_bomba.h"

// Configuración completa de cada tanque
typedef struct {
    const Ultrasonic_Config_t sensor;
    const HAL_Bomba_Config_t bomba;
    const char* nombre;
} Tanque_Config_t;

extern const Tanque_Config_t tanques_config[];
extern const uint32_t NUM_TANQUES;

#endif
