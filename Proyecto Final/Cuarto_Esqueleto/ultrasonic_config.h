#ifndef ULTRASONIC_CONFIG_H
#define ULTRASONIC_CONFIG_H

#include "hal_ultrasonic.h"

// Definición de los puertos y pines para cada sensor
static const MCAL_GPIO_Port_t PORTA_INFO = {PORTA, GPIOA, MCAL_PORTA};
static const MCAL_GPIO_Port_t PORTD_INFO = {PORTD, GPIOD, MCAL_PORTD};

// Configuración de los sensores ultrasónicos
static const Ultrasonic_Config_t ultrasonic_config[] = {
    // Trigger: PORTD.1, Echo: PORTA.12 (Naranja)
    {&PORTD_INFO, 1, &PORTA_INFO, 12},
    // Trigger: PORTD.2, Echo: PORTA.13 (Uva)
    {&PORTD_INFO, 2, &PORTA_INFO, 13},
    // Trigger: PORTD.3, Echo: PORTA.14 (Fresa)
    {&PORTD_INFO, 3, &PORTA_INFO, 14}
};

#define NUM_ULTRASONIC_SENSORS (sizeof(ultrasonic_config)/sizeof(Ultrasonic_Config_t))

#endif // ULTRASONIC_CONFIG_H