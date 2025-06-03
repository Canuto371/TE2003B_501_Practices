// hal_ultrasonic.h
#ifndef HAL_ULTRASONIC_H
#define HAL_ULTRASONIC_H

#include "mcal_ultrasonic.h"
#include <stdint.h>

typedef MCAL_Ultrasonic_Config_t Ultrasonic_Config_t;

void HAL_Ultrasonic_Init(const Ultrasonic_Config_t *config, uint32_t num_sensors);
uint32_t HAL_Ultrasonic_GetDistance(uint8_t sensor_index);

#endif // HAL_ULTRASONIC_H
