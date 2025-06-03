// mcal_ultrasonic.h
#ifndef MCAL_ULTRASONIC_H
#define MCAL_ULTRASONIC_H

#include "mcal_gpio.h"
#include <stdint.h>

typedef struct {
    const MCAL_GPIO_Port_t *trigger_port;
    uint32_t trigger_pin;
    const MCAL_GPIO_Port_t *echo_port;
    uint32_t echo_pin;
} MCAL_Ultrasonic_Config_t;

void MCAL_Ultrasonic_Init(const MCAL_Ultrasonic_Config_t *config);
uint32_t MCAL_Ultrasonic_MeasureDistance(const MCAL_Ultrasonic_Config_t *config);

#endif // MCAL_ULTRASONIC_H
