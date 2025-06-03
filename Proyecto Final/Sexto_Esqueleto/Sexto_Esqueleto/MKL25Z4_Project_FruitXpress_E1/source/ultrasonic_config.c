#include "ultrasonic_config.h"
#include "mcal_gpio.h"

static const MCAL_GPIO_Port_t PORTA_INFO = {PORTA, GPIOA, MCAL_PORTA};
static const MCAL_GPIO_Port_t PORTD_INFO = {PORTD, GPIOD, MCAL_PORTD};

const Tanque_Config_t tanques_config[] = {
    { // Tanque Naranja
        .sensor = {&PORTD_INFO, 1, &PORTA_INFO, 12},
        .bomba = {&PORTA_INFO, 1}, // PTA1
        .nombre = "Naranja"
    },
    { // Tanque Uva
        .sensor = {&PORTD_INFO, 2, &PORTA_INFO, 13},
        .bomba = {&PORTA_INFO, 2}, // PTA2
        .nombre = "Uva"
    },
    { // Tanque Fresa
        .sensor = {&PORTD_INFO, 3, &PORTA_INFO, 14},
        .bomba = {&PORTA_INFO, 3}, // PTA3
        .nombre = "Fresa"
    }
};

const uint32_t NUM_TANQUES = sizeof(tanques_config)/sizeof(Tanque_Config_t);
