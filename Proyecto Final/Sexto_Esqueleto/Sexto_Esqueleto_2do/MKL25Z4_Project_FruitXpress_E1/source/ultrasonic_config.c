#include "ultrasonic_config.h"
#include "mcal_gpio.h"

static const MCAL_GPIO_Port_t PORTA_INFO = {PORTA, GPIOA, MCAL_PORTA};
static const MCAL_GPIO_Port_t PORTD_INFO = {PORTD, GPIOD, MCAL_PORTD};

const Tanque_Config_t tanques_config[] = {
    { // Tanque Naranja (índice 0)
        .sensor = {&PORTD_INFO, 1, &PORTA_INFO, 12},
        .bomba_idx = 0,  // Corresponde a bombas_config[0]
        .nombre = "Naranja"
    },
    { // Tanque Uva (índice 1)
        .sensor = {&PORTD_INFO, 2, &PORTA_INFO, 13},
        .bomba_idx = 1,  // Corresponde a bombas_config[1]
        .nombre = "Uva"
    },
    { // Tanque Fresa (índice 2)
        .sensor = {&PORTD_INFO, 3, &PORTA_INFO, 14},
        .bomba_idx = 2,  // Corresponde a bombas_config[2]
        .nombre = "Fresa"
    }
};

const uint32_t NUM_TANQUES = sizeof(tanques_config)/sizeof(Tanque_Config_t);
